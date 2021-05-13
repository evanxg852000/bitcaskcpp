cmake_minimum_required(VERSION 3.9)
project(bitcaskcpp)

file(GLOB_RECURSE SOURCE_FILES "./*.cpp")

include_directories(${VENDOR_DIR})
include_directories(${INCLUDE_DIR})

add_library(bitcaskcpp SHARED STATIC ${SOURCE_FILES})
target_link_libraries(bitcaskcpp ${CONAN_LIBS})

install(TARGETS bitcaskcpp DESTINATION ${BUILD_FOLDER})
