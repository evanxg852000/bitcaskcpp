cmake_minimum_required(VERSION 3.9)

add_subdirectory(cli)
add_subdirectory(server)

include_directories(${THIRD_PARTY_DIR})
include_directories(${INCLUDE_DIR})