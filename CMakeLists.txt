# xo-distribution/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_distribution VERSION 1.0)

include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options()

#add_subdirectory(example)
add_subdirectory(src/distribution)  # note refcnt dep -> not header-only
add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

#install(Targets ex1 DESTINATION bin/distribution/example)
