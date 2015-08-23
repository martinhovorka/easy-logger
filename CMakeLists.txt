cmake_minimum_required (VERSION 2.8)
project (libel)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-O0 -ggdb3 -Wall -Wextra -ansi -pedantic -std=c99 -D NAMESPACE_EASYLOGGER -D NDEBUG")

message("++ using compiler ${CMAKE_C_COMPILER}")
message("++ using current C99 flags ${CMAKE_C_FLAGS}")

add_executable(libel easylogger.c)
target_link_libraries(libel pthread)

