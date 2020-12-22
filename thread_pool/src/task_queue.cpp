#include "task_queue.hpp"

namespace dts {

task_queue::task_type task_queue::poll() {
    std::unique_lock<std::mutex> ulock(mtx_);
    cv_.wait(ulock, [this]() {
        return !accept_push_ || !q_.empty();
    });
    if (!accept_push_ && q_.empty()) {
        throw thread_pool_stopped("task_queue is empty and task_queue::poll() "
                                  "called after stopping push.");
    }
    task_type task = std::move(q_.front());
    q_.pop();
    return task;
}

void task_queue::stop_push() {
    {
        std::unique_lock<std::mutex> ulock(mtx_);
        accept_push_ = false;
    }
    cv_.notify_all();
}

}  // namespace dts
