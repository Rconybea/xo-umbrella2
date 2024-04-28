# indentlog/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(indentlog VERSION 1.0)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=debug
xo_toplevel_debug_config2()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage
xo_toplevel_coverage_config2()

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)

# header-only library.
# see [[https://stackoverflow.com/questions/47718485/install-and-export-interface-only-library-cmake]]
#
set(SELF_LIB indentlog)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

install(TARGETS hello DESTINATION bin/indentlog/example)
install(TARGETS indentlog_ex1 DESTINATION bin/indentlog/example)
install(TARGETS indentlog_ex2 DESTINATION bin/indentlog/example)
install(TARGETS indentlog_ex3 DESTINATION bin/indentlog/example)
install(TARGETS indentlog_ex4 DESTINATION bin/indentlog/example)
