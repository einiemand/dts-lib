#pragma once

#include <functional>

#include "task_queue.hpp"

namespace dts {

class thread_pool {
public:
    using task_type = typename task_queue::task_type;
    using size_type = typename task_queue::size_type;

    thread_pool(size_type worker_cnt);
    ~thread_pool();

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    template<typename Fn, typename... Args>
    decltype(auto) submit(Fn&& fn, Args&&... args) {
        using fn_res_t = std::invoke_result_t<Fn, Args...>;
        std::packaged_task<fn_res_t()> ptask(
          std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
        auto future = ptask.get_future();
        tq_.emplace(std::move(ptask));
        return future;
    }

private:
    task_queue tq_;
    std::vector<std::thread> workers_;

    void worker_func();
};

}  // namespace dts
