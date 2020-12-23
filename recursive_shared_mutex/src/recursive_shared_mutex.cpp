#include "recursive_shared_mutex.hpp"

namespace dts {

void recursive_shared_mutex::lock() {
    std::thread::id this_id = std::this_thread::get_id();
    std::unique_lock<std::mutex> ulock(mtx_);
    if (this_id == writer_id_) {
        // Same thread/writer trying to acquire the shared_mutex again.
        // Simply increase writer count.
        ++writer_cnt_;
    }
    else {
        // Another writer or no writer is holding the shared_mutex.
        // It's also likely that some readers are holding
        // the shared_mutex.
        if (writer_id_.has_value()) {
            // If another writer is holding the shared_mutex,
            // waiting for it to release.
            cv_.wait(ulock, [this] {
                return writer_cnt_ == 0 && reader_map_.empty();
            });
        }
        std::shared_mutex::lock();
        writer_id_ = this_id;
        writer_cnt_ = 1;
    }
}

bool recursive_shared_mutex::try_lock() {
    std::thread::id this_id = std::this_thread::get_id();
    std::lock_guard<std::mutex> lkgrd(mtx_);
    if (this_id == writer_id_) {
        // Same thread/writer trying to acquire the shared_mutex again.
        // Simply increase writer count.
        ++writer_cnt_;
        return true;
    }
    else if (writer_id_.has_value()) {
        // Another writer is holding the shared_mutex.
        return false;
    }
    else {
        // No writer is holding the shared_mutex but some readers
        // may be.
        bool locked = std::shared_mutex::try_lock();
        if (locked) {
            // Set writer attributes if successfully locked.
            writer_id_ = this_id;
            writer_cnt_ = 1;
        }
        return locked;
    }
}

void recursive_shared_mutex::unlock() {
    std::unique_lock<std::mutex> ulock(mtx_);
    if (!writer_id_.has_value()) {
        // No writer is holding the shared_mutex.
        // Call unlock() anyway. UB expected.
        std::shared_mutex::unlock();
    }
    else {
        // At this point, the shared_mutex must be held by a writer
        // and writer_cnt_ must be greater than 0.
        --writer_cnt_;
        if (writer_cnt_ == 0) {
            // If writer count becomes 0, release the
            // underlying shared_mutex.
            // It's likely that we are unlocking in a
            // different thread from the one where the shared_mutex
            // has been acquired.
            // Call unlock() anyway. UB expected.
            std::shared_mutex::unlock();
            writer_id_ = std::nullopt;
            // Manually unlock to let cv notify.
            ulock.unlock();
            cv_.notify_one();
        }
    }
}

}  // namespace dts
