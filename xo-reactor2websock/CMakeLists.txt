# xo-reactor2websock/CMakeLists.txt
#
# Adapter between xo-reactor (event sources/sinks) and xo-websock (websocket
# transport).  Read the name as reactor-TO-websock, not as xo-reactor2.
#
# Exists so that NEITHER of those depends on the other: xo-websock carries
# frames and speaks only TaggedPtr, and xo-reactor no longer knows the web
# exists.  See .xo-backlog/xo-websock/issues/02.

cmake_minimum_required(VERSION 3.10)

project(reactor2websock VERSION 1.0)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# common c++ settings

# PROJECT_CXX_FLAGS: bespoke for this project - usually empty
set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# sources

add_subdirectory(src/reactor2websock)
add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
