set (THREAD_POOL_TEST_SOURCE
        thread_pool_test.cpp
        )

add_executable (thread_pool_test ${THREAD_POOL_TEST_SOURCE})
target_link_libraries (thread_pool_test dts_thread_pool)
