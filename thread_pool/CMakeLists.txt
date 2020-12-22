cmake_minimum_required (VERSION 3.0)

project (thread_pool)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -pedantic -pthread")

include_directories (include)

add_subdirectory (include)
add_subdirectory (src)
add_subdirectory (test)
