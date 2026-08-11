# xo-equable2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_equable2 VERSION 1.0)
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
# facet code generation

xo_add_genfacet(
    TARGET xo-equable2-facet-equable
    FACET Equable
    INPUT idl/Equable.json5
    OUTPUT_HPP_DIR include/xo/equable2
    OUTPUT_IMPL_SUBDIR detail
)

install(DIRECTORY idl/
    DESTINATION share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN "*.json5")

# ----------------------------------------------------------------
# output targets

add_subdirectory(src/equable2)
#add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
