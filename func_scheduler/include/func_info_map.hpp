#pragma once

#include <chrono>
#include <future>
#include <map>

namespace dts {

template<typename T>
struct is_time_point : std::false_type {};

template<typename Clock, typename Dur>
struct is_time_point<std::chrono::time_point<Clock, Dur>> : std::true_type {};

template<typename TimePoint, typename Func>
class func_info_map {
public:
    static_assert(is_time_point<TimePoint>::value,
                  "TimePoint must be of type std::chrono::time_point");
    static_assert(std::is_invocable_r<void, Func>::value,
                  "Func must be invocable without any argument and its return "
                  "type must be void");

    using tp_type = TimePoint;
    using func_type = Func;
    using _container_type = std::multimap<tp_type, func_type>;

    class func_info {
    public:
        explicit func_info(typename _container_type::node_type&& node)
            : when_(node.key()),
              func_(std::move(node.mapped())) {}

        ~func_info() = default;

        func_info(const func_info&) = delete;
        func_info& operator=(const func_info&) = delete;

        tp_type when() const noexcept {
            return when_;
        }

        func_type& func() noexcept {
            return func_;
        }

    private:
        const tp_type when_;
        func_type func_;
    };

    func_info_map() = default;
    ~func_info_map() = default;

    func_info_map(const func_info_map&) = delete;
    func_info_map& operator=(const func_info_map&) = delete;

    template<typename... Args>
    void emplace(Args&&... args) {
        tfmap_.emplace(std::forward<Args>(args)...);
    }

    bool empty() const noexcept {
        return tfmap_.empty();
    }

    tp_type soonest_invoke_time() const {
        return tfmap_.begin()->first;
    }

    func_info extract_first_info() {
        return func_info(tfmap_.extract(tfmap_.begin()));
    }

private:
    // time_point -> function. A time_point can map to multiple functions.
    _container_type tfmap_;
};

}  // namespace dts
