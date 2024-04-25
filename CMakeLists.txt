# xo-flatstring/CMakeLists.txt

cmake_minimum_required(VERSION 3.25)

project(xo_flatstring VERSION 1.0)
#enable_language(CXX)

# common XO cmake macros (see proj/xo-cmake)
include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options2()

# ----------------------------------------------------------------
# unit test + coverage setup
#
enable_testing()

# ----------------------------------------------------------------
# cmake -DCMAKE_BUILD_TYPE=coverage
#
xo_toplevel_coverage_config2()

# ----------------------------------------------------------------
# c++ settings

# one-time project-specific c++ flags.  usually empty
set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")
add_definitions(${PROJECT_CXX_FLAGS})

xo_toplevel_compile_options()

# ----------------------------------------------------------------

add_subdirectory(example)
add_subdirectory(utest)
add_subdirectory(docs)

# ----------------------------------------------------------------
# provide find_package() support for projects using this library

set(SELF_LIB xo_flatstring)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# dependencies

#xo_headeronly_dependency(${SELF_LIB} randomgen)
# etc..

# end CMakeLists.txt
