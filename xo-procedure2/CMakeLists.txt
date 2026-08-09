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
# facet packages whose interfaces we generate implementations for.
#
# xo_add_genfacetimpl() silently skips (STATUS message only, no target) unless
# share_<FACET_PKG> already exists at the point of the call, so these must be
# found FIRST.  Without them the targets do not exist and regenerating fails
# with a bare "No rule to make target".
#
# These two are sufficient for every genfacetimpl here, including the
# FACET_PKG xo_procedure2 entries: the finds below establish share_* for the
# transitive closure, so no self-referential case is left skipped.  Verified by
# reconfiguring and counting "not available; skipping" -- expect zero.

find_package(xo_printable2 CONFIG REQUIRED)
find_package(xo_alloc2 CONFIG REQUIRED)

# ----------------------------------------------------------------
# output targets

# note: manual target; generated code committed to git
xo_add_genfacet(
    TARGET xo-procedure2-facet-procedure
    FACET Procedure
    INPUT idl/Procedure.json5
    )

# note: manual target; generated code committed to git
xo_add_genfacet(
    TARGET xo-procedure2-facet-runtimecontext
    FACET RuntimeContext
    INPUT idl/RuntimeContext.json5
    )

# ----------------------------------------------------------------

xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-runtimecontext-simplercx
    FACET_PKG xo_procedure2
    INPUT idl/IRuntimeContext_DSimpleRcx.json5
    )

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_0
    FACET_PKG xo_procedure2
    INPUT idl/IProcedure_DPrimitive_gco_0.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_0
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DPrimitive_gco_0.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_0
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DPrimitive_gco_0.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_1_gco
    FACET_PKG xo_procedure2
    INPUT idl/IProcedure_DPrimitive_gco_1_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_1_gco
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DPrimitive_gco_1_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_1_gco
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DPrimitive_gco_1_gco.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_2_gco_gco
    FACET_PKG xo_procedure2
    INPUT idl/IProcedure_DPrimitive_gco_2_gco_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_2_gco_gco
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DPrimitive_gco_2_gco_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_2_gco_gco
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DPrimitive_gco_2_gco_gco.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_2_dict_string
    FACET_PKG xo_procedure2
    INPUT idl/IProcedure_DPrimitive_gco_2_dict_string.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_2_dict_string
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DPrimitive_gco_2_dict_string.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_2_dict_string
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DPrimitive_gco_2_dict_string.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-procedure-primitive_gco_3_dict_string_gco
    FACET_PKG xo_procedure2
    INPUT idl/IProcedure_DPrimitive_gco_3_dict_string_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-gcobject-primitive_gco_3_dict_string_gco
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DPrimitive_gco_3_dict_string_gco.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-procedure2-facetimpl-printable-primitive_gco_3_dict_string_gco
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DPrimitive_gco_3_dict_string_gco.json5
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-procedure2-genfacet-all)

# ----------------------------------------------------------------

add_subdirectory(src/procedure2)
add_subdirectory(utest)

# end CMakeLists.txt
