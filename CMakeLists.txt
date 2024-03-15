# xo-statistics/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_statistics VERSION 1.0)
enable_language(CXX)

include(cmake/xo-bootstrap-macros.cmake)

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
# bespoke (usually temporary) c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# common include paths etc.

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# external dependencies
#
# set CMAKE_INSTALL_PREFIX to analog of /usr
# to use .cmake assistants from /usr/lib/cmake/indentlog
#
# xo_dependency(..)

# ----------------------------------------------------------------

#add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# output targets

set(SELF_LIB xo_statistics)
xo_add_headeronly_library(${SELF_LIB})

# ----------------------------------------------------------------
# standard install + provide find_package() support

xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install additional components

#install(TARGETS statistics_ex1 DESTINATION bin/xo-statistics/example)
