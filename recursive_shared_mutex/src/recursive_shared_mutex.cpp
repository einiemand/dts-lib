#include "recursive_shared_mutex.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"

static const thread_local std::thread::id this_id = std::this_thread::get_id();

namespace dts {

void recursive_shared_mutex::lock() {
    std::unique_lock<std::mutex> ulock(mtx_);
    /**
     * Use of cv_ guarantees that this function doesn't hold mtx_ forever if it
     * can't lock successfully.
     */
    cv_.wait(ulock, [this] {
        return try_lock_in_this_thread();
    });
    writer_id_ = this_id;
    ++writer_cnt_;
}

bool recursive_shared_mutex::try_lock() {
    std::lock_guard<std::mutex> lkgrd(mtx_);
    bool locked = try_lock_in_this_thread();
    if (locked) {
        writer_id_ = this_id;
        ++writer_cnt_;
    }
    return locked;
}

void recursive_shared_mutex::unlock() {
    {
        std::lock_guard<std::mutex> lkgrd(mtx_);
        if (--writer_cnt_ == 0) {
            // Current thread is not a writer any more.
            std::shared_mutex::unlock();
            writer_id_ = std::nullopt;
            if (!reader_map_.empty()) {
                // If current thread is a reader, lock_shared().
                std::shared_mutex::lock_shared();
            }
        }
    }
    cv_.notify_all();
}

void recursive_shared_mutex::lock_shared() {
    std::unique_lock<std::mutex> ulock(mtx_);
    /**
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

bool recursive_shared_mutex::try_lock_in_this_thread() {
    /**
     * Return true if
     * Current thread is already a writer
     * or
     * There is no writer/reader and current thread successfully becomes a
     * writer.
     *
     * This is a private member function. Doesn't need to be protected by mutex.
     */
    return this_id == writer_id_ || !writer_id_.has_value() &&
                                      reader_map_.empty() &&
                                      std::shared_mutex::try_lock();
}

bool recursive_shared_mutex::try_lock_shared_in_this_thread() {
    /**
     * Return true if
     * Current thread is already a writer
     * or
     * There is no writer and current thread is/becomes a reader.
     *
     * It's likely that current thread is already a reader ->
     * reader_map_.find(this_id) != reader_map_.end(). In this case, we don't
     * need to call try_lock_shared().
     *
     * This is a private member function. Doesn't need to be protected by mutex.
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

#pragma GCC diagnostic pop
