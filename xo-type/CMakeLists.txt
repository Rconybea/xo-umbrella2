# xo-type/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_type VERSION 1.0)
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
    TARGET xo-type-facet-type
    FACET Type
    INPUT idl/Type.json5
    OUTPUT_HPP_DIR include/xo/type
    OUTPUT_IMPL_SUBDIR type
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-type-atomictype
    FACET_PKG xo_type
    INPUT idl/IType_DAtomicType.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-gcobject-atomictype
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DAtomicType.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-type-listtype
    FACET_PKG xo_type
    INPUT idl/IType_DListType.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-gcobject-listtype
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DListType.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-type-arraytype
    FACET_PKG xo_type
    INPUT idl/IType_DArrayType.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-gcobject-arraytype
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DArrayType.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-type-functiontype
    FACET_PKG xo_type
    INPUT idl/IType_DFunctionType.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-gcobject-functiontype
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DFunctionType.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-type-typevarref
    FACET_PKG xo_type
    INPUT idl/IType_DTypeVarRef.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-type-facetimpl-gcobject-typevarref
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DTypeVarRef.json5
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-type-genfacet-all)

install(DIRECTORY idl/
    DESTINATION share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN "*.json5")

# ----------------------------------------------------------------
# output targets

add_subdirectory(src/type)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
