project (func_scheduler)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-g -pthread -Wall -Wextra -pedantic")

include_directories (include)

add_subdirectory (include)
add_subdirectory (src)
add_subdirectory (test)
