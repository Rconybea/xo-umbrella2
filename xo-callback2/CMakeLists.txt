# xo-callback2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_callback2 VERSION 1.0)
enable_language(CXX)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags. usually empty
set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------
# output targets

add_subdirectory(utest)

# ----------------------------------------------------------------
# header-only library

set(SELF_LIB xo_callback2)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-callback2/cmake/xo_callback2Config.cmake.in

xo_headeronly_dependency(${SELF_LIB} xo_alloc2)
xo_headeronly_dependency(${SELF_LIB} callback)
xo_headeronly_dependency(${SELF_LIB} xo_facet)

# NB must follow the xo_*dependency() calls above: xo_export_cmake_config()
#    snapshots the xo_deps target property to generate
#    @XO_FIND_DEPENDENCY_BLOCK@ in the Config.cmake
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
