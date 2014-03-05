#include "coroutine.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

using namespace std;

class Coro
{
public:
    Coro(const CoroFunc_t& func, int stackSize, bool isMain);
    ~Coro();

    CoroFunc_t func;

    CoroStatus status;

    ucontext_t context;

    Coro *back;

    char* stack;
};

Coro::Coro(const CoroFunc_t& f, int stackSize, bool isMain)
    : func(f)
    , status(Coro_Ready)
    , back(0)
{
    if(!isMain)
    {
        int pageSize = getpagesize();
        char* buf = (char*)malloc(stackSize + pageSize);

        mprotect(buf, pageSize, PROT_NONE);

        stack = buf + pageSize;
    }
    else 
    {
        stack = 0;
    }
}


Coro::~Coro()
{
    if(stack)
    {
        int pageSize = getpagesize();
        char* buf = stack - pageSize;

        mprotect(buf, pageSize, PROT_READ | PROT_WRITE);
        free(buf);
    }
}

Coroutine::Coroutine(int stackSize)
    : m_stackSize(stackSize)
{
    m_mainCoro = new Coro(CoroFunc_t(), 0, true);
    m_mainCoro->status = Coro_Running;

    m_runningCoro = m_mainCoro;
}

Coroutine::~Coroutine()
{
    for(auto i = m_coros.begin(); i != m_coros.end(); ++i)
    {
        delete *i;
    }

    delete m_mainCoro;
}

Coro_t Coroutine::create(const CoroFunc_t& func)
{
    Coro* c = new Coro(func, m_stackSize, false);

    m_coros.insert(c);

    return c;
}

void Coroutine::corofunc(uint32_t low32, uint32_t hi32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    Coroutine* mgr = (Coroutine*)ptr;

    Coro* c = mgr->m_runningCoro;

    c->func();

    mgr->m_coros.erase(c);

    c->status = Coro_Dead;

    swapcontext(&c->context, &c->back->context);
}

int Coroutine::resume(Coro_t coro)
{
    auto i = m_coros.find(coro);
    if(i == m_coros.end())
    {
        return -1;
    }

    Coro* c = *i;
    switch(c->status)
    {
        case Coro_Ready:
        {
            getcontext(&c->context);
            c->context.uc_stack.ss_sp = c->stack;
            c->context.uc_stack.ss_size = m_stackSize;  
            c->context.uc_link = NULL; //never return from corofunc
            
            uintptr_t ptr = (uintptr_t)this;
            makecontext(&c->context, (void (*)(void))Coroutine::corofunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));  
            break;
        }
        case Coro_Suspended:
            break;
        default:
            return -1;
    }

    c->back = m_runningCoro;
    c->status = Coro_Running;
   
    m_runningCoro->status = Coro_Normal;

    m_runningCoro = c;

    swapcontext(&c->back->context, &c->context);
    
    m_runningCoro = c->back;
    m_runningCoro->status = Coro_Running;

    if(c->status == Coro_Dead)
    {
        delete c;
    }
    return 0;
}

int Coroutine::yield() 
{
    if(m_runningCoro == m_mainCoro)
    {
        return -1;
    }

    Coro* c = m_runningCoro;
    c->status = Coro_Suspended;

    swapcontext(&c->context, &c->back->context);
    return 0;
}

Coro_t Coroutine::running()
{
    return (m_runningCoro == m_mainCoro) ? 0 : m_runningCoro; 
}

CoroStatus Coroutine::status(Coro_t coro)
{
    auto it = m_coros.find(coro);
    if(it == m_coros.end())
    {
        return Coro_Dead; 
    }

    return (*it)->status;
}
