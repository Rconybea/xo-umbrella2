# xo-ordinaltree/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_ordinaltree VERSION 0.1)
enable_language(CXX)

# common XO macros (see github:Rconybea/xo-cmake)
include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# output targets

add_subdirectory(utest)

# ----------------------------------------------------------------
# header-only library

set(SELF_LIB xo_ordinaltree)
xo_add_headeronly_library(${SELF_LIB})

# ----------------------------------------------------------------
#
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
# (note: ..Targets from xo_install_library2())
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# input dependencies

# NOTE: dependency set here must be kept consistent with ordinaltree/cmake/xo_ordinaltreeConfig.cmake.in

# xo-ordinaltree is also header-only
xo_headeronly_dependency(${SELF_LIB} randomgen)
