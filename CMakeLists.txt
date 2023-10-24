# xo-pyprocess/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyprocess VERSION 0.1)

include(xo_macros/xo-project-macros)

xo_cxx_toplevel_options()

add_subdirectory(src/pyprocess)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)
