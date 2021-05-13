cmake_minimum_required(VERSION 3.9)
project(bitcaskpp)

file(GLOB_RECURSE SOURCE_FILES "./*.cpp")

include_directories(${VENDOR_DIR})
include_directories(${INCLUDE_DIR})

add_library(bitcaskpp SHARED STATIC ${SOURCE_FILES})
target_link_libraries(bitcaskpp ${CONAN_LIBS})

install(TARGETS bitcaskpp DESTINATION ${BUILD_FOLDER})
