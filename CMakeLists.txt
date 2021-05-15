cmake_minimum_required(VERSION 3.9)
project(CPP_PROJECT)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
include("./cmake.modules/macros.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})


set(BUILD_FOLDER "./build")
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(APPS_DIR "${PROJECT_SOURCE_DIR}/apps")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third_party")


include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)
add_subdirectory(benchmarks)

enable_testing()
