#include "phasephdgr/locks.hpp"

scoped_simple_lock::scoped_simple_lock(simple_lock * l) : l(l) { l->lock(); }
scoped_simple_lock::~scoped_simple_lock() { l->unlock(); }

void simple_lock::lock(){ while (flag.test_and_set(memory_order_acquire)); }
void simple_lock::unlock(){ flag.clear(std::memory_order_release); }
scoped_simple_lock simple_lock::make_scoped_lock() { return scoped_simple_lock(this); }
