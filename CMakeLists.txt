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
message("-- PROJECT_CXX_FLAGS_DEBUG: debug c++ flags are [${PROJECT_CXX_FLAGS_DEBUG}]")

add_compile_options("$<$<CONFIG:DEBUG>:${PROJECT_CXX_FLAGS_DEBUG}>")

# ----------------------------------------------------------------
# unit test setup

# activate code coverage for all executables + libraries (when configured with -DCODE_COVERAGE=ON)
add_code_coverage()
# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
# NOTE: this seems to work only with the 'ccov-all' target.  In particular,  doesn't seem to do anything with the 'ccov' target
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* ${PROJECT_SOURCE_DIR}/utest/* ${PROJECT_BINARY_DIR}/local/* ${PROJECT_SOURCE_DIR}/repo/*)

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
