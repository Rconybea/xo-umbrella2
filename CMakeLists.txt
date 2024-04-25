# xo-unit/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_unit VERSION 1.0)

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
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "-Wstringop-overread")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)

# ----------------------------------------------------------------
# provide find_package() support for projects using this library

set(SELF_LIB xo_unit)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on all the other library/utest targets
#
add_subdirectory(docs)

# ----------------------------------------------------------------
# dependencies

xo_headeronly_dependency(${SELF_LIB} xo_ratio)
# etc..

# end CMakeLists.txt
