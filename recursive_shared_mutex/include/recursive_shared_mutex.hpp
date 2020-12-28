#pragma once

#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace dts {

class recursive_shared_mutex : private std::shared_mutex {
public:
    recursive_shared_mutex() = default;
    ~recursive_shared_mutex() = default;

    void lock();
    bool try_lock();
    void unlock();

    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();

private:
    std::mutex mtx_;
    std::condition_variable cv_;

    std::optional<std::thread::id> writer_id_ = std::nullopt;
    std::size_t writer_cnt_ = 0;

    std::unordered_map<std::thread::id, std::size_t> reader_map_;

    bool try_lock_shared_in_this_thread();
};

}  // namespace dts
