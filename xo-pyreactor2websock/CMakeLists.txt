# xo-pyreactor2websock/CMakeLists.txt
#
# python bindings for xo-reactor2websock -- module xo.reactor2websock.
# See .xo-backlog/xo-websock/issues/02.

cmake_minimum_required(VERSION 3.10)
project(xo_pyreactor2websock VERSION 1.0)

include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

add_subdirectory(src/pyreactor2websock)
add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# ${CMAKE_BINARY_DIR}/xo-python: python3 with this build's pybind11 module, and
# its already-installed dependencies, on PYTHONPATH.  The utest runs through it.
#
# Only for a standalone build.  In the umbrella the single top-level wrapper
# covers every module; a per-subsystem one there would list whatever happened to
# be registered so far, which is worse than none.  Same as xo-pyobject2.

xo_establish_submodule_build()

if(NOT XO_SUBMODULE_BUILD)
    xo_emit_python_wrapper(${CMAKE_BINARY_DIR})
endif()
