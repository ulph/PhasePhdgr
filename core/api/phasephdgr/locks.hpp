#pragma once

#include <atomic>

using namespace std;

class simple_lock;

class scoped_simple_lock {
    simple_lock * l;
public:
    scoped_simple_lock(simple_lock * l);
    ~scoped_simple_lock();
};

class simple_lock {
    friend class scoped_simple_lock;
private:
    atomic_flag flag = ATOMIC_FLAG_INIT;
    void lock();
    void unlock();
public:
    scoped_simple_lock make_scoped_lock();
};
