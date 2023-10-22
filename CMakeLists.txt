# pyutil/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyutil VERSION 0.1)
enable_language(CXX)

# common XO cmake macros (see github:Rconybea/xo-cmake)
include(xo_macros/xo_cxx)
include(xo_macros/code-coverage)   # very little to unit test here

# ----------------------------------------------------------------
# unit test setup

enable_testing()
## enable code coverage for all executables+libraries
## (when configured with -DCODE_COVERAGE=ON)
##
add_code_coverage()
add_code_coverage_all_targets(
  EXCLUDE
  /nix/store/*
  ${PROJECT_SOURCE_DIR}/utest/*)

# ----------------------------------------------------------------
# bespoke (usually temporary) c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# c++ settings

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

set(SELF_LIB xo_pyutil)
xo_add_headeronly_library(${SELF_LIB})
#xo_include_headeronly_options2(${SELF_LIB})

# ----------------------------------------------------------------
# standard install + provide find_package() support

xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
#xo_install_include_tree()
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install any additional components

#install(TARGETS ex1 DESTINATION bin/${PROJECT_NAME}/example)
