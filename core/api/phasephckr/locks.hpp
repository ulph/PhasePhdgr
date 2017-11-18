#pragma once

#include <atomic>

using namespace std;

class mutex;

class scoped_lock {
    mutex * m;
public:
    scoped_lock(mutex * m);
    ~scoped_lock();
};

class mutex {
private:
    atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock();
    void unlock();
    scoped_lock make_scoped_lock();
};
