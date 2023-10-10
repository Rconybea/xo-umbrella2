# callback/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(callback VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

# ----------------------------------------------------------------
# unit test setup (for consistency with other xo libraries.  no unit tests as of 10oct2023)

enable_testing()
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

# PROJECT_CXX_FLAGS: bespoke for this project - usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# sources

add_subdirectory(src/callback)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support to customers

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
