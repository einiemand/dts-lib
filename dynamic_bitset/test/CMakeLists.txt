set (DYNAMIC_BITSET_TEST_SOURCE
        dynamic_bitset_test.cpp
        )

add_executable (dynamic_bitset_test ${DYNAMIC_BITSET_TEST_SOURCE})
target_link_libraries (dynamic_bitset_test dts_dynamic_bitset)
