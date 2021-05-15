cmake_minimum_required(VERSION 3.9)
project(benchmarks)

include_directories(${THIRD_PARTY_DIR})
include_directories(${INCLUDE_DIR})

set(PROJECTS_SOURCE_FILES "")

subdirlist(PROJECTS_MODULES ${SOURCE_DIR})
foreach(MODULE_DIR ${PROJECTS_MODULES})
    if(MODULE_DIR STREQUAL "main")
        continue()
    endif()
    set(files "")
    file(GLOB_RECURSE files "${SOURCE_DIR}/${MODULE_DIR}/*.cc")
    list(APPEND PROJECTS_SOURCE_FILES ${files})
endforeach()

file(GLOB_RECURSE BENCHMARKS_SOURCE_FILES "./*.cc")

add_executable(benchmarks ${BENCHMARKS_SOURCE_FILES} ${PROJECTS_SOURCE_FILES})
target_link_libraries(benchmarks  bitcaskcpp ${CONAN_LIBS})
install(TARGETS benchmarks DESTINATION ${BUILD_FOLDER})
