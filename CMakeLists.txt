# xo-distribution/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_distribution VERSION 1.0)

# common XO cmake macros (see github.com:Rconybea/xo-cmake)
include(xo_macros/xo-project-macros)

xo_cxx_toplevel_options()

#add_subdirectory(example)
#add_subdirectory(uteest)

xo_add_headeronly_library4(xo_distribution ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

#install(Targets ex1 DESTINATION bin/distribution/example)
