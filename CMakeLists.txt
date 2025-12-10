# xo/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo-umbrella VERSION 1.0)

# ----------------------------------------------------------------
# global build settings

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

# Adopting submodule builds directly into this cmake.
# Submodule builds will pickup dependent xo artifacts directly
# from sibling build dirs.
# (Contrast with a build that relies on install step).
# In particular,  configure step in satellite projects
# needs to avoid using cmake find_package() on sibling xo projects:
# 1. .cmake support files
#      fooConfig.cmake
#      fooConfigVersion.cmake
#      fooTargets.cmake
#    won't have been installed
# 2. In any case,  they point to final install location;
#    we need build location
#
set(XO_SUBMODULE_BUILD True)

# toplevel source directory;  used only with XO_SUBMODULE_BUILD
set(XO_UMBRELLA_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(XO_UMBRELLA_REPO_SUBDIR .)
# toplevel binary directory;  used only with XO_SUBMODULE_BUILD
set(XO_UMBRELLA_BINARY_DIR ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/xo-cmake/cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------

# temporary compiler flags here
set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "runpath for installed libraries/executables")
endif()

message("-- CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
message("-- CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("-- CMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}")
message("-- XO_ENABLE_DOCS=${XO_ENABLE_DOCS}")
message("-- XO_ENABLE_EXAMPLES=${XO_ENABLE_EXAMPLES}")
message("-- XO_ENABLE_VULKAN=${XO_ENABLE_VULKAN}")
message("-- XO_ENABLE_OPENGL=${XO_ENABLE_OPENGL}")

# ----------------------------------------------------------------
#
set(DOX_EXCLUDE_PATTERNS [=[
*/utest/* \
*/llvm/* \
*/llvm-c/*
]=])

# ----------------------------------------------------------------
# xo satellite projects
# in reverse topological order i.e. dependencies first

add_subdirectory(xo-cmake)
add_subdirectory(xo-indentlog)
add_subdirectory(xo-allocutil)
add_subdirectory(xo-refcnt)
add_subdirectory(xo-subsys)
add_subdirectory(xo-randomgen)
add_subdirectory(xo-flatstring)
add_subdirectory(xo-pyutil)
add_subdirectory(xo-reflectutil)
add_subdirectory(xo-reflect)
add_subdirectory(xo-pyreflect)
add_subdirectory(xo-ratio)
add_subdirectory(xo-unit)
add_subdirectory(xo-pyunit)
add_subdirectory(xo-callback)
add_subdirectory(xo-alloc)
add_subdirectory(xo-alloc2)  # experiment w/ sep iface,data
add_subdirectory(xo-object)
add_subdirectory(xo-ordinaltree)
#
add_subdirectory(xo-webutil)
add_subdirectory(xo-pywebutil)
add_subdirectory(xo-printjson)
add_subdirectory(xo-pyprintjson)
add_subdirectory(xo-reactor)
add_subdirectory(xo-pyreactor)
add_subdirectory(xo-websock)
add_subdirectory(xo-pywebsock)
#
add_subdirectory(xo-statistics)
add_subdirectory(xo-distribution)
add_subdirectory(xo-pydistribution)
add_subdirectory(xo-simulator)
add_subdirectory(xo-pysimulator)
add_subdirectory(xo-process)
add_subdirectory(xo-pyprocess)
add_subdirectory(xo-kalmanfilter)
add_subdirectory(xo-pykalmanfilter)
#
add_subdirectory(xo-expression)
add_subdirectory(xo-pyexpression)
add_subdirectory(xo-tokenizer)
add_subdirectory(xo-reader)
#add_subdirectory(xo-symboltable)
add_subdirectory(xo-interpreter)
add_subdirectory(xo-jit)
add_subdirectory(xo-pyjit)
#
add_subdirectory(xo-imgui)

# ----------------------------------------------------------------
# documentation.  must follow add_subdirectory() for satellite projects

xo_umbrella_doxygen_deps(xo_facet xo_alloc indentlog xo_flatstring xo_ratio xo_unit xo_tokenizer xo_reader xo_interpreter xo_jit)
xo_umbrella_doxygen_config()
xo_umbrella_sphinx_config(index.rst docs/install.rst docs/glossary.rst)
