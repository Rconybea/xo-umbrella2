# xo-webutil/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(webutil VERSION 1.0)
enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(cmake/xo-bootstrap-macros.cmake)

# ----------------------------------------------------------------
# unit test setup (no unit tests yet,  but want 'make tests' to do something)

enable_testing()
# activate code coverage for all executables + libraries (when -DCODE_COVERAGE=ON)
add_code_coverage()

# 1. assuming that /nix/store/ prefixes .hpp files belonging to gcc, catch2 etc.
#    we're not interested in code coverage for these sources.
# 2. exclude the utest/ subdir, we don't need coverage on the unit tests themselves;
#    rather,  want coverage on the code that the unit tests exercise.
#
add_code_coverage_all_targets(EXCLUDE /nix/store/* utest/*)

# ----------------------------------------------------------------
# common c++ settings

# PROJECT_CXX_FLAGS: bespoke for this project - usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

xo_toplevel_compile_options()

# ----------------------------------------------------------------
# sources

add_subdirectory(src/webutil)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
