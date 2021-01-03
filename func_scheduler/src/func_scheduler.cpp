#include "func_scheduler.hpp"

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"

namespace dts {

func_scheduler::func_scheduler(std::size_t worker_cnt) {
    while (worker_cnt--) {
        workers_.emplace_back([this] {
            worker_func();
        });
    }
}

func_scheduler::~func_scheduler() {
    {
        std::unique_lock<std::mutex> ulock(mtx_);
        if (accept_new_) {
            wait_impl(ulock);
        }
    }
    dispatcher_.join();
    for (auto& worker : workers_) {
        worker.join();
    }
}

void func_scheduler::wait() {
    std::unique_lock<std::mutex> ulock(mtx_);
    wait_impl(ulock);
}

void func_scheduler::dispatch_func() {
    while (true) {
        {
            std::unique_lock<std::mutex> ulock(mtx_);
            if (todo_.empty()) {
                dispatch_cv_.wait(ulock, [this] {
                    return !accept_new_ || !todo_.empty();
                });
                if (!accept_new_) {
                    break;
                }
            }
            auto invoke_time = todo_.begin()->first;
            while (invoke_time > clock_type::now()) {
                bool pred = dispatch_cv_.wait_until(ulock, invoke_time, [&] {
                    return todo_.begin()->first < invoke_time;
                });
                if (pred) {
                    invoke_time = todo_.begin()->first;
                }
            }
        }
        worker_cv_.notify_one();
    }
}

void func_scheduler::worker_func() {
    while (true) {
        func_type func;
        {
            std::unique_lock<std::mutex> ulock(mtx_);
            worker_cv_.wait(ulock, [this] {
                return !accept_new_ ||
                       !todo_.empty() &&
                         todo_.cbegin()->first <= clock_type::now();
            });
            if (!accept_new_ && todo_.empty()) {
                break;
            }
            auto func_info = todo_.extract(todo_.begin());
            func = std::move(func_info.mapped());
        }
        std::invoke(func);
        all_done_cv_.notify_one();
    }
}

void func_scheduler::wait_impl(std::unique_lock<std::mutex>& ulock) {
    accept_new_ = false;
    all_done_cv_.wait(ulock, [this] {
        return todo_.empty();
    });
    ulock.unlock();
    dispatch_cv_.notify_one();
    worker_cv_.notify_all();
}

}  // namespace dts

#pragma GCC diagnostic pop
