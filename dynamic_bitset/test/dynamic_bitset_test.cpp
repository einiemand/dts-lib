#include "dynamic_bitset.hpp"

#include <bitset>
#include <iostream>

using dts::dynamic_bitset;
using dts::uint64_width;

void test_to_string() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width> bs(num);
        assert(db.to_string() == bs.to_string());
    }
}

void test_set() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width> bs(num);
        std::size_t pos = rand() % uint64_width;
        std::size_t len = rand() % (uint64_width - pos);
        bool val = rand() % 2;
        db.set(pos, len, val);
        for (std::size_t j = pos; j < pos + len; ++j) {
            bs.set(j, val);
        }
        assert(db.to_string() == bs.to_string());
    }
}

void test_left_shift() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        std::size_t offset = rand() % (uint64_width + 1);
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width> bs(num);
        db <<= offset;
        bs <<= offset;
        assert(db.to_string() == bs.to_string());
    }
}

void test_right_shift() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        std::size_t offset = rand() % (uint64_width + 1);
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width> bs(num);
        db >>= offset;
        bs >>= offset;
        assert(db.to_string() == bs.to_string());
    }
}

void test_flip() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width> bs(num);
        db.flip();
        bs.flip();
        assert(db.to_string() == bs.to_string());
    }
}

void test_push_back() {
    static constexpr std::size_t iter_cnt = 100;
    for (std::size_t i = 0; i < iter_cnt; ++i) {
        uint64_t num = rand();
        dynamic_bitset db(uint64_width, num);
        std::bitset<uint64_width + 1> bs(num);
        bool bit = rand() % 2;
        db.push_back(bit);
        bs[uint64_width] = bit;
        assert(db.to_string() == bs.to_string());
    }
}

int main() {
    srand(time(0));
    test_to_string();
    test_set();
    test_left_shift();
    test_right_shift();
    test_flip();
    test_push_back();
    std::cout << "All tests passed\n";

    return 0;
}