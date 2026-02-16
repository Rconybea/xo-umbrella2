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

# note: manual target; generated code committed to git
xo_add_genfacet(
    TARGET xo-object2-facet-sequence
    FACET Sequence
    INPUT idl/Sequence.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR sequence
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-sequence-list
    FACET_PKG xo_object2
    FACET Sequence
    REPR List
    INPUT idl/ISequence_DList.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR list
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-list
    FACET_PKG xo_printable2
    FACET Printable
    REPR List
    INPUT idl/IPrintable_DList.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR list
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-list
    FACET_PKG xo_gc
    FACET GCObject
    REPR List
    INPUT idl/IGCObject_DList.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR list
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-boolean
    FACET_PKG xo_printable2
    FACET Printable
    REPR Boolean
    INPUT idl/IPrintable_DBoolean.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR boolean
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-boolean
    FACET_PKG xo_gc
    FACET GCObject
    REPR Boolean
    INPUT idl/IGCObject_DBoolean.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR boolean
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-float
    FACET_PKG xo_printable2
    FACET Printable
    REPR Float
    INPUT idl/IPrintable_DFloat.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR number
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-float
    FACET_PKG xo_gc
    FACET GCObject
    REPR Float
    INPUT idl/IGCObject_DFloat.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR number
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-integer
    FACET_PKG xo_printable2
    FACET Printable
    REPR Integer
    INPUT idl/IPrintable_DInteger.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR number
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-integer
    FACET_PKG xo_gc
    FACET GCObject
    REPR Integer
    INPUT idl/IGCObject_DInteger.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR number
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-string
    FACET_PKG xo_printable2
    FACET Printable
    REPR String
    INPUT idl/IPrintable_DString.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR string
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-string
    FACET_PKG xo_gc
    FACET GCObject
    REPR String
    INPUT idl/IGCObject_DString.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR string
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-sequence-array
    FACET_PKG xo_object2
    FACET Sequence
    REPR Array
    INPUT idl/ISequence_DArray.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR array
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-array
    FACET_PKG xo_printable2
    FACET Printable
    REPR Array
    INPUT idl/IPrintable_DArray.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR array
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-array
    FACET_PKG xo_gc
    FACET GCObject
    REPR Array
    INPUT idl/IGCObject_DArray.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR array
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-runtimeerror
    FACET_PKG xo_printable2
    FACET Printable
    REPR RuntimeError
    INPUT idl/IPrintable_DRuntimeError.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR error
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-runtimeerror
    FACET_PKG xo_gc
    FACET GCObject
    REPR RuntimeError
    INPUT idl/IGCObject_DRuntimeError.json5
    OUTPUT_HPP_DIR include/xo/object2
    OUTPUT_IMPL_SUBDIR error
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-object2-genfacet-all)

# ----------------------------------------------------------------

# must complete definition of expression lib before configuring examples
add_subdirectory(src/object2)
add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end CMakeLists.txt
