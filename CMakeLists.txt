# xo-pykalmanfilter/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pykalmanfilter VERSION 1.0)

include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options()

add_subdirectory(src/pykalmanfilter)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
