#include "recursive_shared_mutex.hpp"

#include <cassert>
#include <iostream>

using dts::recursive_shared_mutex;

template<template<typename> typename Lock>
void test_recursiveness(recursive_shared_mutex& rsmtx, int x) {
    if (x > 0) {
        Lock<recursive_shared_mutex> lk(rsmtx);
        test_recursiveness<Lock>(rsmtx, x - 1);
    }
}

void test_recursiveness_shared() {
    recursive_shared_mutex rsmtx;
    test_recursiveness<std::shared_lock>(rsmtx, 10);
}

void test_recursiveness_unique() {
    recursive_shared_mutex rsmtx;
    test_recursiveness<std::unique_lock>(rsmtx, 10);
}

void test_try_lock() {
    recursive_shared_mutex rsmtx;
    assert(rsmtx.try_lock());
    assert(rsmtx.try_lock());

    std::thread other_writer([&rsmtx] {
        assert(!rsmtx.try_lock());
    });
    other_writer.join();

    rsmtx.unlock();
    rsmtx.unlock();
}

int main() {
    test_recursiveness_shared();
    test_recursiveness_unique();
    test_try_lock();

    std::cout << "All tests passed\n";

    return 0;
}