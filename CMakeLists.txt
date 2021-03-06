cmake_minimum_required(VERSION 2.6)

project(nb)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(LIBTNT_PATH "libtarantool path")

## build options
#
find_path(TARANTOOL_INCLUDE_DIR tarantool/tnt.h "${LIBTNT_PATH}/include")
if (NOT TARANTOOL_INCLUDE_DIR)
	message(FATAL_ERROR 
	"Tarantool DB client library header not found")
endif()

find_library(TARANTOOL_LIB tarantool PATH ${LIBTNT_PATH}/tnt)
find_library(TARANTOOLNET_LIB tarantoolnet PATH ${LIBTNT_PATH}/tntnet)
find_library(TARANTOOLSQL_LIB tarantoolsql PATH ${LIBTNT_PATH}/tntsql)

find_library(M_LIB m)

## cflags
##
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -std=gnu99 -Wall -Wextra -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -pg")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")

include_directories("${PROJECT_SOURCE_DIR}/sources")
include_directories("${TARANTOOL_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/third_party/msgpuck")
include_directories("${PROJECT_SOURCE_DIR}/third_party/tarantool-c")

add_definitions("-DTB_LOCAL=${PROJECT_SOURCE_DIR}/third_party/tarantool-c/lib")

add_subdirectory(third_party)
add_subdirectory(sources)
add_subdirectory(plotter)
