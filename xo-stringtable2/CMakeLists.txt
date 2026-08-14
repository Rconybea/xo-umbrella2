# xo-stringtable2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_stringtable2 VERSION 1.0)
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

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-stringtable2-facetimpl-gcobject-string
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DString.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-stringtable2-facetimpl-printable-string
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DString.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-stringtable2-facetimpl-gcobject-uniquestring
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DUniqueString.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-stringtable2-facetimpl-printable-uniquestring
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DUniqueString.json5
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-stringtable2-genfacet-all)

# ----------------------------------------------------------------
# output targets

add_subdirectory(src/stringtable2)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
