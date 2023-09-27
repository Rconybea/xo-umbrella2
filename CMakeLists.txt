# refcnt/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(refcnt VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

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

set(XO_PROJECT_NAME refcnt)
set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")

add_definitions(${PROJECT_CXX_FLAGS})

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

# always write compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# sources

add_subdirectory(src)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install .hpp

xo_install_include_tree()

# end CMakeLists.txt
