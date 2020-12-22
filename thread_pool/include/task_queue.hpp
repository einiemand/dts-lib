#pragma once

#include "thread_pool_stopped.hpp"

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

namespace dts {

class task_queue {
public:
    using task_type = std::packaged_task<void()>;
    using value_type = task_type;
    using size_type = std::queue<task_type>::size_type;

    task_queue() = default;
    ~task_queue() = default;

    task_queue(const task_queue&) = delete;
    task_queue& operator=(const task_queue&) = delete;

    template<typename... Args>
    void emplace(Args&&... args) {
        {
            std::unique_lock<std::mutex> ulock(mtx_);
            if (!accept_push_) {
                throw thread_pool_stopped("task_queue::push() called after stopping push.");
            }
            q_.emplace(std::forward<Args>(args)...);
        }
        cv_.notify_one();
    }

    task_type poll();

    void stop_push();
private:
    std::mutex mtx_;
    std::condition_variable cv_;
    bool accept_push_ = true;
    std::queue<task_type> q_;
};

}
