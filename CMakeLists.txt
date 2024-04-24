# xo-unit/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_unit VERSION 1.0)
enable_testing()
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(cmake/xo-bootstrap-macros.cmake)

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=debug

# clear out hardwired default.
# we want override project-level defaults, so need to prevent interference from hardwired defaults
# (the problem with non-empty hardwired defaults is that we can't tell if they've been set on the
# command line)
#
set(CMAKE_CXX_FLAGS_DEBUG "")

# CMAKE_CXX_FLAGS_DEBUG is built-in to cmake and has non-empty default.
#  -> we cannot tell whether it was set on the command line
#  -> use PROJECT_CXX_FLAGS_DEBUG instead
#
# built-in default value is -g; can hardwire different project policy here
#
if (NOT DEFINED PROJECT_CXX_FLAGS_DEBUG)
    set(PROJECT_CXX_FLAGS_DEBUG ${PROJECT_CXX_FLAGS} -ggdb -Og
        CACHE STRING "debug c++ compiler flags")
endif()
if (${CMAKE_BUILD_TYPE} STREQUAL debug)
    message("-- PROJECT_CXX_FLAGS_DEBUG: debug c++ flags are [${PROJECT_CXX_FLAGS_DEBUG}]")
endif()

add_compile_options("$<$<CONFIG:DEBUG>:${PROJECT_CXX_FLAGS_DEBUG}>")

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage

if (NOT DEFINED PROJECT_CXX_FLAGS_COVERAGE)
    # note: for clang would use -fprofile-instr-generate -fcoverage-mapping here instead and also at link time
    set(PROJECT_CXX_FLAGS_COVERAGE ${PROJECT_CXX_FLAGS} -ggdb -Og -fprofile-arcs -ftest-coverage
        CACHE STRING "coverage c++ compiler flags")
endif()
if (${CMAKE_BUILD_TYPE} STREQUAL "coverage")
    message(STATUS "-- PROJECT_CXX_FLAGS_COVERAGE: coverage c++ flags are [${PROJECT_CXX_FLAGS_COVERAGE}]")
endif()

add_compile_options("$<$<CONFIG:COVERAGE>:${PROJECT_CXX_FLAGS_COVERAGE}>")
# when -DCMAKE_BUILD_TYPE=coverage, link executables with gcov
link_libraries("$<$<CONFIG:COVERAGE>:gcov>")

find_program(LCOV_EXECUTABLE NAMES lcov)
find_program(GENHTML_EXECUTABLE NAMES genhtml)

# with coverage build:
# 1. invoke instrumented executables for which you want coverage:
#     (cd path/to/build && ctest)
# 2. post-process low-level coverage data
#     (path/to/build/gen-ccov)
# 3. point browser to generated html data
#     file:///path/to/build/ccov/html/index.html
#
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/gen-ccov.in
    ${PROJECT_BINARY_DIR}/gen-ccov)

file(CHMOD ${PROJECT_BINARY_DIR}/gen-ccov PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

xo_toplevel_compile_options()

# ----------------------------------------------------------------

#add_subdirectory(src/unit)
add_subdirectory(example)
add_subdirectory(utest)
add_subdirectory(docs)

# ----------------------------------------------------------------
# provide find_package() support for projects using this library

set(SELF_LIB xo_unit)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# dependencies

xo_headeronly_dependency(${SELF_LIB} xo_ratio)
# etc..

# end CMakeLists.txt
