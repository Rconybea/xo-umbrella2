# pyutil/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyutil VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)   # very little to unit test here

xo_cxx_toplevel_options2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage
xo_toplevel_coverage_config2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=debug
xo_toplevel_debug_config2()

# ----------------------------------------------------------------
# bespoke (usually temporary) c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

#add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------

set(SELF_LIB xo_pyutil)
add_library(${SELF_LIB} INTERFACE)
xo_include_headeronly_options2(${SELF_LIB})
xo_install_library2(${SELF_LIB})
xo_install_include_tree3(include/xo/pyutil)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install any additional components

#install(TARGETS ex1 DESTINATION bin/${PROJECT_NAME}/example)
