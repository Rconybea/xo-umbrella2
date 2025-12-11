# xo-facet/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_facet VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")  # gcc-only!
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

# must complete definition of expression lib before configuring examples
#add_subdirectory(src/alloc)
add_subdirectory(utest)
#xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

set(SELF_LIB xo_facet)
xo_add_headeronly_library(${SELF_LIB})
xo_install_library4(${SELF_LIB} ${PROJECT_NAME}Targets)
xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
add_subdirectory(docs)

# end CMakeLists.txt
