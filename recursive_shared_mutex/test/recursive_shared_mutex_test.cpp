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

void test_try_lock_shared() {
    recursive_shared_mutex rsmtx;
    assert(rsmtx.try_lock_shared());
    assert(rsmtx.try_lock_shared());

    std::thread other_writer([&rsmtx] {
        assert(rsmtx.try_lock_shared());
        rsmtx.unlock_shared();
    });
    other_writer.join();

    rsmtx.unlock_shared();
    rsmtx.unlock_shared();
}

void test_read_after_write() {
    recursive_shared_mutex rsmtx;
    std::lock_guard<recursive_shared_mutex> writer(rsmtx);
    assert(rsmtx.try_lock_shared());
    rsmtx.unlock_shared();

    std::thread other_writer([&rsmtx] {
        assert(!rsmtx.try_lock());
    });
    std::thread other_reader([&rsmtx] {
        assert(!rsmtx.try_lock_shared());
    });

    other_writer.join();
    other_reader.join();
}

void test_write_after_read() {
    recursive_shared_mutex rsmtx;
    std::shared_lock<recursive_shared_mutex> reader(rsmtx);
    assert(!rsmtx.try_lock());

    std::thread other_writer([&rsmtx] {
        assert(!rsmtx.try_lock());
    });
    std::thread other_reader([&rsmtx] {
        assert(rsmtx.try_lock_shared());
    });

    other_writer.join();
    other_reader.join();
}

int main() {
    test_recursiveness_shared();
    test_recursiveness_unique();
    test_try_lock();
    test_try_lock_shared();
    test_read_after_write();
    test_write_after_read();

    std::cout << "All tests passed\n";

    return 0;
}