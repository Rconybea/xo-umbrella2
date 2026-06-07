# xo-imgui/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_imgui VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)   # glvnd (for vendor-neutral dispatch) instead of legacy opengl)
endif()

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")  # gcc-only!
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

set(XO_EXAMPLE_INSTALL_DIR bin/xo/imgui/example)

add_subdirectory(src/imgui)
add_subdirectory(example)
# add_subdirectory(utest)

# set(SELF_LIB. ..)
# xo_install_library4(${SELF_LIB} ...)
# xo_export_cmake_config(${PROJECT_NAME} ...)

# ----------------------------------------------------------------
# docs targets depends on other library/utest/exec targets,
# must come after them
#
#add_subdirectory(docs)
