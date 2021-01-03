#include "func_scheduler.hpp"

#include <iostream>

using dts::func_scheduler;

template<typename T>
struct duration_unit_string {};

template<>
struct duration_unit_string<std::chrono::nanoseconds> {
    static const char* const unit;
};

const char* const duration_unit_string<std::chrono::nanoseconds>::unit = "ns";

template<>
struct duration_unit_string<std::chrono::microseconds> {
    static const char* const unit;
};

const char* const duration_unit_string<std::chrono::microseconds>::unit = "us";

template<>
struct duration_unit_string<std::chrono::milliseconds> {
    static const char* const unit;
};

const char* const duration_unit_string<std::chrono::milliseconds>::unit = "ms";

template<>
struct duration_unit_string<std::chrono::seconds> {
    static const char* const unit;
};

const char* const duration_unit_string<std::chrono::seconds>::unit = "s";

static std::mutex cout_mtx;

template<typename Dur>
void test_fs(func_scheduler& fs, const std::string& msg, const Dur& dur) {
    auto before = func_scheduler::clock_type::now();
    auto after_fut = fs.run_after(dur, [] {
        return func_scheduler::clock_type::now();
    });
    auto actual_delay =
      std::chrono::duration_cast<Dur>(after_fut.get() - before);

    std::lock_guard<std::mutex> lkgrd(cout_mtx);
    std::cout << msg << '\n'
              << "expected delay = " << dur.count()
              << duration_unit_string<Dur>::unit
              << ", actual delay = " << actual_delay.count()
              << duration_unit_string<Dur>::unit << '\n';
}

int main() {
    using dts::func_scheduler;
    func_scheduler fs(std::thread::hardware_concurrency());
    auto delay = std::chrono::milliseconds(1000);
    std::vector<std::thread> tests;

    tests.emplace_back([&] {
        test_fs(fs, "Measuring delay in ns",
                std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
    });
    tests.emplace_back([&] {
        test_fs(fs, "Measuring delay in us",
                std::chrono::duration_cast<std::chrono::microseconds>(delay));
    });
    tests.emplace_back([&] {
        test_fs(fs, "Measuring delay in ms",
                std::chrono::duration_cast<std::chrono::milliseconds>(delay));
    });
    tests.emplace_back([&] {
        test_fs(fs, "Measuring delay in s",
                std::chrono::duration_cast<std::chrono::seconds>(delay));
    });

    for (std::thread& test : tests) {
        test.join();
    }

    return 0;
}