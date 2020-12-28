#pragma once

#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace dts {

/*
 * A recursive shared mutex that is similar to ReentrantReadWriteLock in Java.
 * A writer can acquire a read lock but a reader can't acquire a write lock.
 *
 * UB if any of the below actions are performed
 * 1. unlock before it is locked.
 * 2. unlock in a different thread from the one where it has been locked.
 */
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

    bool try_lock_in_this_thread();
    bool try_lock_shared_in_this_thread();
};

}  // namespace dts
