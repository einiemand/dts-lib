set (RSMTX_TEST_SOURCE
        recursive_shared_mutex_test.cpp
        )

add_executable (rsmtx_test ${RSMTX_TEST_SOURCE})
target_link_libraries (rsmtx_test dts_rsmtx)
