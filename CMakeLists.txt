cmake_minimum_required(VERSION 3.10)

project(xo_macros VERSION 1.0)

# if any are useful for this project..
#include (cmake/foo.cmake)

# ----------------------------------------------------------------
# cmake export

set(XO_PROJECT_NAME xo_macros)

install(
    FILES
    "cmake/xo_cxx.cmake"
    DESTINATION share/cmake/${XO_PROJECT_NAME}
)
