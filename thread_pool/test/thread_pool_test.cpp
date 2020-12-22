#include "thread_pool.hpp"

#include <cassert>
#include <iostream>
#include <random>

void test(int iter_cnt, int target) {
    std::cout << "iter_cnt: " << iter_cnt << '\n';
    std::cout << "target: " << target << '\n';
    std::cout << "test progress:\n";
    std::mutex mtx;
    for (int k = 1; k <= iter_cnt; ++k) {
        int v = 0;
        std::vector<std::future<int>> results;
        results.reserve(target);
        dts::thread_pool pool(std::thread::hardware_concurrency());
        for (int i = 0; i < target; ++i) {
            results.emplace_back(pool.submit(
              [&mtx](int& n) {
                  std::unique_lock<std::mutex> ulock(mtx);
                  return ++n;
              },
              std::ref(v)));
        }
        int max_res = 0;
        std::for_each(std::begin(results), std::end(results),
                      [&max_res](auto& result) {
                          max_res = std::max(max_res, result.get());
                      });
        assert(v == target && max_res == target);
        fprintf(stderr, "\r%.2f%%", k * 100.0 / iter_cnt);
    }
    std::cout << '\n';
}

int main() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(500, 1000);
    test(distribution(generator), distribution(generator));

    return 0;
}