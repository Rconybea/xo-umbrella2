# xo-reader/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_reader VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(src/reader)
add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
