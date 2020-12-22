#pragma once

#include <optional>
#include <shared_mutex>
#include <thread>

namespace dts {

class recursive_shared_mutex : private std::shared_mutex {
public:
    recursive_shared_mutex() = default;
    ~recursive_shared_mutex() = default;
    
    void lock();
    bool try_lock();
    void unlock();

    /*
    * std::shared_mutex's shared locking is recursive by default.
    */
    void lock_shared() {
        std::shared_mutex::lock_shared();
    }

    bool try_lock_shared() {
        return std::shared_mutex::try_lock_shared();
    }
    
    void unlock_shared() {
        std::shared_mutex::unlock_shared();
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::optional<std::thread::id> writer_id_ = std::nullopt;
    std::size_t writer_cnt_ = 0;
};

}
