# xo-imgui/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_imgui VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")  # gcc-only!
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(example)
# add_subdirectory(utest)

# set(SELF_LIB. ..)
# xo_install_library4(${SELF_LIB} ...)
# xo_export_cmake_config(${PROJECT_NAME} ...)

if (XO_ENABLE_EXAMPLES)
    install(TARGETS imgui_ex1 DESTINATION bin/imgui/example)
endif()

# ----------------------------------------------------------------
# docs targets depends on other library/utest/exec targets,
# must come after them
#
#add_subdirectory(docs)
