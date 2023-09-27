# using indentlog/CMakeLists.txt as model

cmake_minimum_required(VERSION 3.10)

project(randomgen VERSION 0.1)
enable_language(CXX)

include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)
#include(cmake/cxx.cmake)

# ----------------------------------------------------------------
# unit test setup

enable_testing()
# activate code coverage for all executables + libraries (when configured with -DCODE_COVERAGE=ON)
add_code_coverage()
# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* utest/*)

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")

add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# default install

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# external dependencies
#
# set CMAKE_INSTALL_PREFIX to analog of /usr
# to use .cmake assistants from /usr/lib/cmake/indentlog
#
#find_package(indentlog REQUIRED)

# ----------------------------------------------------------------

add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------

xo_install_include_tree()

install(TARGETS ex1 DESTINATION bin/randomgen/example)
