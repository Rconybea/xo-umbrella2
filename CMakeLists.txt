# xo-procedure2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_procedure2 VERSION 1.0)
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
# output targets

# note: manual target; generated code committed to git
xo_add_genfacet(
    TARGET xo-procedure2-facet-procedure
    FACET Procedure
    INPUT idl/Procedure.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
    )

# note: manual target; generated code committed to git
xo_add_genfacet(
    TARGET xo-procedure2-facet-runtimecontext
    FACET RuntimeContext
    INPUT idl/RuntimeContext.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
    )

# ----------------------------------------------------------------

xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-runtimecontext-simplercx
    FACET_PKG xo_procedure2
    FACET RuntimeContext
    REPR DSimpleRcx
    INPUT idl/IRuntimeContext_DSimpleRcx.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
    )

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_2_gco_gco
    FACET_PKG xo_procedure2
    FACET Procedure
    REPR DPrimitive_gco_2_gco_gco
    INPUT idl/IProcedure_DPrimitive_gco_2_gco_gco.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_2_gco_gco
    FACET_PKG xo_gc
    FACET GCObject
    REPR Primitive_gco_2_gco_gco
    INPUT idl/IGCObject_DPrimitive_gco_2_gco_gco.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_2_gco_gco
    FACET_PKG xo_printable2
    FACET Printable
    REPR Primitive_gco_2_gco_gco
    INPUT idl/IPrintable_DPrimitive_gco_2_gco_gco.json5
    OUTPUT_HPP_DIR include/xo/procedure2
    OUTPUT_IMPL_SUBDIR detail
)

xo_add_genfacet_all(xo-procedure2-genfacet-all)

# ----------------------------------------------------------------

add_subdirectory(src/procedure2)
add_subdirectory(utest)

# ----------------------------------------------------------------
# cmake export

#xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
