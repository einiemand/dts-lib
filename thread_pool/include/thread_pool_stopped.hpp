#pragma once

#include <stdexcept>

namespace dts {

class thread_pool_stopped final : public std::runtime_error {
public:
    thread_pool_stopped(const std::string& msg)
        : std::runtime_error(msg)
    {
    }

    thread_pool_stopped(const char* msg)
        : std::runtime_error(msg)
    {
    }

    ~thread_pool_stopped() override = default;

    thread_pool_stopped(const thread_pool_stopped&) = default;
    thread_pool_stopped& operator=(const thread_pool_stopped&) = default;
};

}
