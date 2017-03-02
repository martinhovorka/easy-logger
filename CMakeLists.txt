#   EasyLogger - Simple thread-safe logging library
#   Copyright (C) 2016, 2017  Martim Hovorka
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.*
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


cmake_minimum_required (VERSION 2.8)
project (el)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-O3 -Wall -Wextra -ansi -pedantic -std=c99 -D NAMESPACE_EASYLOGGER -D NDEBUG")

message("++ using compiler       : ${CMAKE_C_COMPILER}")
message("++ using following flags: ${CMAKE_C_FLAGS}")

add_library(el easylogger.c)
target_link_libraries(el pthread)

