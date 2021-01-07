#include "func_scheduler.hpp"

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"

namespace dts {

func_scheduler::func_scheduler(std::size_t worker_cnt) {
    while (worker_cnt--) {
        workers_.emplace_back([this] {
            work_func();
        });
    }
}

func_scheduler::~func_scheduler() {
    wait();
    dispatcher_.join();
    for (auto& worker : workers_) {
        worker.join();
    }
}

void func_scheduler::wait() {
    {
        std::unique_lock<std::mutex> ulock(mtx_);
        if (!accept_new_) {
            // wait() has already been called.
            return;
        }
        accept_new_ = false;
        all_done_cv_.wait(ulock, [this] {
            return todo_.empty();
        });
    }
    dispatch_cv_.notify_one();
    worker_cv_.notify_all();
}

void func_scheduler::dispatch_func() {
    while (true) {
        {
            std::unique_lock<std::mutex> ulock(mtx_);
            if (todo_.empty()) {
                dispatch_cv_.wait(ulock, [this] {
                    return !accept_new_ || !todo_.empty();
                });
                if (!accept_new_ && todo_.empty()) {
                    break;
                }
            }
            auto invoke_time = todo_.soonest_invoke_time();
            while (invoke_time > clock_type::now()) {
                bool has_sooner_invoke_time =
                  dispatch_cv_.wait_until(ulock, invoke_time, [&] {
                      return todo_.soonest_invoke_time() < invoke_time;
                  });
                if (has_sooner_invoke_time) {
                    invoke_time = todo_.soonest_invoke_time();
                }
            }
        }
        worker_cv_.notify_one();
    }
}

void func_scheduler::work_func() {
    while (true) {
        func_type func;
        {
            std::unique_lock<std::mutex> ulock(mtx_);
            worker_cv_.wait(ulock, [this] {
                return !accept_new_ ||
                       !todo_.empty() &&
                         todo_.soonest_invoke_time() <= clock_type::now();
            });
            if (!accept_new_ && todo_.empty()) {
                break;
            }
            auto func_info = todo_.extract_first_info();
            func = std::move(func_info.func());
        }
        std::invoke(func);
        all_done_cv_.notify_one();
    }
}

}  // namespace dts

#pragma GCC diagnostic pop
