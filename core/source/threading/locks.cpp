#include "phasephckr/locks.hpp"

scoped_lock::scoped_lock(mutex * m) : m(m) { m->lock(); }
scoped_lock::~scoped_lock() { m->unlock(); }

void mutex::lock()
{
    while (flag.test_and_set(memory_order_acquire));
}

void mutex::unlock()
{
    flag.clear(std::memory_order_release);
}

scoped_lock mutex::make_scoped_lock() { return scoped_lock(this); }
