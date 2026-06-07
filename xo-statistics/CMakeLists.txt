# xo-statistics/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_statistics VERSION 1.0)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# bespoke (usually temporary) c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

#add_subdirectory(example)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# output targets

set(SELF_LIB xo_statistics)
xo_add_headeronly_library(${SELF_LIB})

# ----------------------------------------------------------------
# standard install + provide find_package() support

xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# install additional components

#install(TARGETS statistics_ex1 DESTINATION bin/xo-statistics/example)
