# xo-pyunit/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyunit VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage
xo_toplevel_coverage_config2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=debug
xo_toplevel_debug_config2()

# ----------------------------------------------------------------
# c++ settings (usually temporary)

set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(src/pyunit)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
