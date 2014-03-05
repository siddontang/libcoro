#pragma once

#include <stdint.h>
#include <memory>
#include <functional>
#include <ucontext.h>
#include <set>

//default coroutine stack size 64k
#define DEFAULT_CORO_STACKSIZE 65536

enum CoroStatus 
{
    //coroutine is running
    Coro_Running,

    //coroutine is created but has not started
    Coro_Ready,

    //coroutine is yield
    Coro_Suspended,

    //coroutine is active but not running (that is ,it has resumed another coroutine)
    Coro_Normal,

    //coroutine has finished
    Coro_Dead,
};

class Coro;

typedef Coro* Coro_t;
typedef std::function<void (void)> CoroFunc_t;

class Coroutine 
{
public:
    Coroutine(int stackSize = DEFAULT_CORO_STACKSIZE);
    ~Coroutine();

    //create a new coroutine with func
    Coro_t create(const CoroFunc_t& func);
    
    //start or continue the execution of coroutine coro
    //return < 0 for error
    int resume(Coro_t coro);

    //suspend the execution of calling coroutin
    //return < 0 for error
    int yield();

    //return status of coro
    CoroStatus status(Coro_t coro);

    //return current running coroutin or 0 when called by main thread
    Coro_t running();

private:
    static void corofunc(uint32_t low32, uint32_t hi32); 

private:
    std::set<Coro_t> m_coros;

    int m_stackSize;

    Coro* m_runningCoro;

    Coro* m_mainCoro;
};

static Coroutine coroutine;
