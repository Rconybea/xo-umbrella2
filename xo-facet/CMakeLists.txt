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

add_subdirectory(src/facet)

# must complete definition of expression lib before configuring examples
add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------

# Install the generator script:
install(
    PROGRAMS codegen/genfacet
    DESTINATION share/xo-facet/codegen
    COMPONENT codegen
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/genfacet-bin
"#!/bin/bash
exec \"${CMAKE_INSTALL_FULL_DATADIR}/xo-facet/codegen/genfacet\" \"\$@\"
")

install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/genfacet-bin
    DESTINATION bin
    RENAME genfacet
)

# Install all .j2 template files
install(
    DIRECTORY codegen/
    DESTINATION share/xo-facet/codegen
    COMPONENT codegen
    FILES_MATCHING PATTERN "*.j2"
)

# ----------------------------------------------------------------

# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
add_subdirectory(docs)

# end CMakeLists.txt
