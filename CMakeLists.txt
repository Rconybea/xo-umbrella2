# xo-ordinaltree/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_ordinaltree VERSION 0.1)
enable_language(CXX)

# common XO macros (see github:Rconybea/xo-cmake)
include(cmake/xo-bootstrap-macros.cmake)

# ----------------------------------------------------------------
# unit test setup

enable_testing()
# enable code coverage for all executables+libraries
# (when configured with -DCODE_COVERAGE=ON)
#
add_code_coverage()
add_code_coverage_all_targets(
  EXCLUDE
  /nix/store/*
  ${PROJECT_SOURCE_DIR}/utest/*)

# ----------------------------------------------------------------
# c++ settings

# sets XO_COMPILE_OPTIONS
xo_toplevel_compile_options()

# ----------------------------------------------------------------
# output targets

add_subdirectory(utest)

# ----------------------------------------------------------------
# header-only library

set(SELF_LIB xo_ordinaltree)
xo_add_headeronly_library(${SELF_LIB})

# ----------------------------------------------------------------
#
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
# (note: ..Targets from xo_install_library2())
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# input dependencies

# NOTE: dependency set here must be kept consistent with ordinaltree/cmake/xo_ordinaltreeConfig.cmake.in

# xo-ordinaltree is also header-only
xo_headeronly_dependency(${SELF_LIB} randomgen)
