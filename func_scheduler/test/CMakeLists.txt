set (FSCHEDULER_TEST_SOURCE
        func_scheduler_test.cpp
        )

add_executable (fscheduler_test ${FSCHEDULER_TEST_SOURCE})
target_link_libraries (fscheduler_test dts_fscheduler)
