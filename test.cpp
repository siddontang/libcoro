#include "coroutine.h"

#include <iostream>

using namespace std;

Coro_t co1, co2;

void func1()
{
    int a = 10;
    cout << "hello func1" << endl;
    
    Coro_t co = coroutine.running();
    if(co != co1)
    {
        cout << "error, co1 must run" << endl;
        exit(1);
    }
    
    coroutine.yield();

    co = coroutine.running();
    if(co != co1)
    {
        cout << "error, co1 must run" << endl;
        exit(1);
    }
    

    cout << "hello end func1\t" << a << endl;
}

void func2(Coro_t id)
{
    int a = 100;
    cout << "hello func2" << endl;

    Coro_t co = coroutine.running();
    if(co != co2)
    {
        cout << "error, co2 must run" << endl;
        exit(1);
    }
 
    CoroStatus s = coroutine.status(co1);
    if(s != Coro_Suspended)
    {
        cout << "error, co1 must suspeneded" << endl;
        exit(1);
    }

    coroutine.resume(id);

    s = coroutine.status(co1);
    if(s != Coro_Dead)
    {
        cout << "error, co1 must daed" << endl;
        exit(1);
    }

    co = coroutine.running();
    if(co != co2)
    {
        cout << "error, co2 must run" << endl;
        exit(1);
    }
 

    coroutine.yield();

    cout << "hello end func2\t" << a << endl;
}

void func()
{
    cout << "hello func" << endl;

    co1 = coroutine.create(std::bind(&func1));

    coroutine.resume(co1);

    co2 = coroutine.create(std::bind(&func2, co1));

    coroutine.resume(co2);

    coroutine.resume(co2);

    CoroStatus s = coroutine.status(co2);
    if(s != Coro_Dead)
    {
        cout << "error, co2 must dead" << endl;
        exit(1);
    }

    cout << "hello end fun" << endl;
}

int main()
{    
    Coro_t co = coroutine.create(std::bind(&func));
    
    coroutine.resume(co);
    
    cout << "end" << endl;

    return 0;
}
