#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <mutex>

namespace dts {

class func_scheduler {
public:
    using clock_type = std::chrono::steady_clock;
    using tp_type =
      std::chrono::time_point<clock_type, std::chrono::nanoseconds>;
    using func_type = std::packaged_task<void()>;

    func_scheduler(std::size_t worker_cnt);
    ~func_scheduler();

    func_scheduler(const func_scheduler&) = delete;
    func_scheduler& operator=(const func_scheduler&) = delete;

    template<typename Rep, typename Period, typename Fn, typename... Args>
    decltype(auto) run_after(const std::chrono::duration<Rep, Period>& dur,
                             Fn&& fn, Args&&... args) {
        const tp_type now = clock_type::now();
        return run_at(now + dur, std::forward<Fn>(fn),
                      std::forward<Args>(args)...);
    }

    template<typename Rep, typename Period, typename Fn, typename... Args>
    decltype(auto) run_at(
      const std::chrono::time_point<clock_type,
                                    std::chrono::duration<Rep, Period>>& when,
      Fn&& fn, Args&&... args) {
        const tp_type now = clock_type::now();
        if (when <= now) {
            throw std::runtime_error("bad");
        }
        using fn_res_t = std::invoke_result_t<Fn, Args...>;
        std::packaged_task<fn_res_t()> ptask(
          std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
        auto future = ptask.get_future();
        {
            std::lock_guard<std::mutex> lkgrd(mtx_);
            todo_.emplace(when, std::move(ptask));
        }
        dispatch_cv_.notify_one();
        return future;
    }

    void wait();

private:
    std::mutex mtx_;
    std::condition_variable all_done_cv_;
    std::condition_variable dispatch_cv_;
    std::condition_variable worker_cv_;

    bool accept_new_ = true;
    std::thread dispatcher_{ [this] {
        dispatch_func();
    } };
    std::vector<std::thread> workers_;
    std::multimap<tp_type, func_type> todo_;

    void dispatch_func();
    void worker_func();

    void wait_impl(std::unique_lock<std::mutex>& ulock);
};

}  // namespace dts
