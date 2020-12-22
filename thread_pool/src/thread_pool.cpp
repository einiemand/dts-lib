#include "thread_pool.hpp"

namespace dts {

thread_pool::thread_pool(size_type worker_cnt)
    : tq_(),
      workers_()
{
    while (worker_cnt--) {
        workers_.emplace_back([this]() { worker_func(); });
    }
}

thread_pool::~thread_pool() {
    tq_.stop_push();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

void thread_pool::worker_func() {
    task_type task;
    while (true) {
        try {
            task = tq_.poll();
        } catch (thread_pool_stopped&) {
            break;
        }
        std::invoke(task);
    }
}

}
