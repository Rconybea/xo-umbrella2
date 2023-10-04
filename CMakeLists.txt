# xo-tree/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_tree VERSION 0.1)
enable_language(CXX)

# common XO macros (see github:Rconybea/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)

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

set(SELF_LIB xo_tree)

add_library(${SELF_LIB} INTERFACE)
xo_include_headeronly_options2(${SELF_LIB})

# ----------------------------------------------------------------
#
xo_install_library2(${PROJECT_NAME})
xo_install_include_tree()
# (note: ..Targets from xo_install_library2())
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# input dependencies

xo_dependency_headeronly(${SELF_LIB} randomgen)

