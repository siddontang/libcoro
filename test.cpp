#include "coroutine.h"

#include <iostream>

using namespace std;

void func1()
{
    int a = 10;
    cout << "hello func1" << endl;
    coroutine.yield();

    cout << "hello end func1\t" << a << endl;
}

void func2(Coro_t id)
{
    int a = 100;
    cout << "hello func2" << endl;

    coroutine.resume(id);

    coroutine.yield();

    cout << "hello end func2\t" << a << endl;
}

void func()
{
    cout << "hello func" << endl;

    Coro_t id1 = coroutine.create(std::bind(&func1));

    coroutine.resume(id1);

    Coro_t id2 = coroutine.create(std::bind(&func2, id1));

    coroutine.resume(id2);

    coroutine.resume(id2);

    cout << "hello end fun" << endl;
}

int main()
{    
    Coro_t id = coroutine.create(std::bind(&func));
    
    coroutine.resume(id);
    
    cout << "end" << endl;

    return 0;
}
