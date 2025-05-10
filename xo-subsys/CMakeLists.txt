# xo-subsys/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(subsys VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage
xo_toplevel_coverage_config2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=debug
xo_toplevel_debug_config2()


#set(XO_PROJECT_NAME subsys)

# ----------------------------------------------------------------
# installing header-only library

set(SELF_LIB subsys)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
#add_library(subsys INTERFACE)
#xo_include_headeronly_options2(subsys)
#xo_install_library2(subsys)

# ----------------------------------------------------------------
# provide find_package() support

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
