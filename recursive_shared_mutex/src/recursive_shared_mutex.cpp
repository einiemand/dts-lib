#include "recursive_shared_mutex.hpp"

static const thread_local std::thread::id this_id = std::this_thread::get_id();

namespace dts {

void recursive_shared_mutex::lock() {
    std::unique_lock<std::mutex> ulock(mtx_);
    /*
     * Proceed only if
     * Current thread is already a writer
     * or
     * There is no writer or reader, and shared_mutex::try_lock() returns
     * true.
     *
     * Use of cv_ guarantees that this function doesn't hold mtx_ forever if it
     * can't lock successfully.
     */
    cv_.wait(ulock, [this] {
        return this_id == writer_id_ || !writer_id_.has_value() &&
                                          reader_map_.empty() &&
                                          std::shared_mutex::try_lock();
    });
    writer_id_ = this_id;
    ++writer_cnt_;
}

bool recursive_shared_mutex::try_lock() {
    std::lock_guard<std::mutex> lkgrd(mtx_);
    if (this_id == writer_id_) {
        /*
         * Same thread/writer trying to acquire the shared_mutex again. Simply
         * increase writer count.
         */
        ++writer_cnt_;
        return true;
    }
    else if (writer_id_.has_value()) {
        // Another writer is holding the shared_mutex.
        return false;
    }
    else {
        // No writer is holding the shared_mutex but some readers may be.
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
        /*
         * No writer is holding the shared_mutex. Call unlock() anyway. UB
         * expected.
         */
        std::shared_mutex::unlock();
    }
    else {
        /*
         * At this point, the shared_mutex must be held by a writer and
         * writer_cnt_ must be greater than 0.
         */
        --writer_cnt_;
        if (writer_cnt_ == 0) {
            /*
             * If writer count becomes 0, release the underlying shared_mutex.
             * It's likely that user is unlocking in a different thread from the
             * one where the shared_mutex has been acquired. Call unlock()
             * anyway. UB expected.
             */
            std::shared_mutex::unlock();
            writer_id_ = std::nullopt;
            // Handle the situation where there are readers in current thread
            if (!reader_map_.empty()) {
                /*
                 * If there are readers, they must be in current thread. If they
                 * are not, UB.
                 */
                std::shared_mutex::lock_shared();
            }
            // Manually unlock to let cv notify.
            ulock.unlock();
            cv_.notify_all();
        }
    }
}

void recursive_shared_mutex::lock_shared() {
    std::unique_lock<std::mutex> ulock(mtx_);
    /*
     * Use of cv_ guarantees that this function doesn't hold mtx_ forever if it
     * can't lock_shared successfully.
     */
    cv_.wait(ulock, [this] {
        return try_lock_shared_in_this_thread();
    });
    // Update reader_map_.
    ++reader_map_[this_id];
}

bool recursive_shared_mutex::try_lock_shared() {
    std::lock_guard<std::mutex> lkgrd(mtx_);
    bool shared_locked = try_lock_shared_in_this_thread();
    if (shared_locked) {
        ++reader_map_[this_id];
    }
    return shared_locked;
}

/*
 * This function will be long and hard to understand if we take into account all
 * unexpected behaviors, e.g. unlock_shared in a different thread and
 * unlock_shared on an exclusive lock, etc. So we just presume callers wouldn't
 * do anything wrong. User should expect UB if otherwise.
 */
void recursive_shared_mutex::unlock_shared() {
    {
        std::lock_guard<std::mutex> ulock(mtx_);
        // There must be at least one reader in current thread.
        auto iter = reader_map_.find(this_id);
        if (--(iter->second) == 0) {
            // If this is the last reader in current thread.
            if (!writer_id_.has_value()) {
                // If current thread is not a writer either.
                std::shared_mutex::unlock_shared();
            }
            reader_map_.erase(iter);
        }
    }
    cv_.notify_all();
}

bool recursive_shared_mutex::try_lock_shared_in_this_thread() {
    /*
     * Return true if
     * Current thread is already a writer
     * or
     * There is no writer and current thread is/becomes a reader.
     *
     * It's likely that current thread is already a reader ->
     * reader_map_.find(this_id) != reader_map_.end(). In this case, we don't
     * need to call try_lock_shared().
     *
     * This is a private member function. No need to be protected by mutex.
     */
    return this_id == writer_id_ ||
           !writer_id_.has_value() &&
             (reader_map_.find(this_id) != reader_map_.end() ||
              std::shared_mutex::try_lock_shared());

    // bool shared_locked = false;
    // if (this_id == writer_id_) {
    //     // Current thread is a writer. Ofc a writer can read too.
    //     shared_locked = true;
    // }
    // else if (!writer_id_.has_value()) {
    //     // There's no writer at the moment.
    //     if (reader_map_.find(this_id) != reader_map_.end() ||
    //         std::shared_mutex::try_lock_shared())
    //     {
    //         // Current thread is already a reader or try_lock_shared()
    //         returns
    //         // true.
    //         shared_locked = true;
    //     }
    // }
    // return shared_locked;
}

}  // namespace dts
