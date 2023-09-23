# using indentlog/CMakeLists.txt as model

cmake_minimum_required(VERSION 3.10)

project(randomgen VERSION 0.1)
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
# default install

if(NOT USER)
    set(USER $ENV{USER})
endif()

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /home/${USER}/local CACHE STRING "install directory")
endif()
if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH /home/${USER}/local/lib CACHE STRING "runpath in installed libraries/executables")
endif()

# ----------------------------------------------------------------
# external dependencies
#
# set CMAKE_INSTALL_PREFIX to analog of /usr
# to use .cmake assistants from /usr/lib/cmake/indentlog
#
find_package(indentlog REQUIRED)

# ----------------------------------------------------------------

add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

install(TARGETS ex1 DESTINATION bin/randomgen/example)
