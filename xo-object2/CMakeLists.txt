# xo-object2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_object2 VERSION 0.1)

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
    TARGET xo-object2-facet-sequence
    FACET Sequence
    INPUT idl/Sequence.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR sequence
    OUTPUT_CPP_DIR src/object2
)

# ----------------------------------------------------------------

# must complete definition of expression lib before configuring examples
add_subdirectory(src/object2)
#add_subdirectory(utest)
#xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end CMakeLists.txt
