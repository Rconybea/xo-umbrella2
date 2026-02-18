# xo-printable2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_printable2 VERSION 0.1)

include(GNUInstallDirs)
include(cmake/xo-bootstrap-macros.cmake)

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------
# c++ settings

set(PROJECT_CXX_FLAGS "")
#set(PROJECT_CXX_FLAGS "-fconcepts-diagnostics-depth=2")  # gcc-only!
add_definitions(${PROJECT_CXX_FLAGS})

# ----------------------------------------------------------------

xo_add_genfacet(
    TARGET xo-printable2-facet-printable
    FACET Printable
    INPUT idl/Printable.json5
    OUTPUT_HPP_DIR include/xo/printable2
    OUTPUT_IMPL_SUBDIR detail
)

# ----------------------------------------------------------------

add_subdirectory(src/printable2)
#add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end CMakeLists.txt
