# refcnt/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo-refcnt VERSION 0.1)
enable_language(CXX)

include(cmake/cxx.cmake)
include(cmake/code-coverage.cmake)

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

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# ----------------------------------------------------------------
# sources

add_subdirectory(src)

# ----------------------------------------------------------------
# install .hpp

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

# end CMakeLists.txt
