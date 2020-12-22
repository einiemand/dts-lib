set (THREAD_POOL_SOURCES
        task_queue.cpp
        thread_pool.cpp
        )

add_library (dts_thread_pool ${THREAD_POOL_HEADERS} ${THREAD_POOL_SOURCES})
