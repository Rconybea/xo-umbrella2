# xo-pyarena/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_pyarena VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings (usually temporary)

set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# sources

add_subdirectory(src/pyarena)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# ${CMAKE_BINARY_DIR}/xo-python: python3 with this build's pybind11 module, and
# its already-installed dependencies, on PYTHONPATH.
#
# Only for a standalone build.  In the umbrella the single top-level wrapper
# covers every module; a per-subsystem one there would list whatever happened to
# be registered so far, which is worse than none.

xo_establish_submodule_build()

if(NOT XO_SUBMODULE_BUILD)
    xo_emit_python_wrapper(${CMAKE_BINARY_DIR})
endif()

# end CMakeLists.txt
