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
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-sequence-list
    FACET_PKG xo_object2
    INPUT idl/ISequence_DList.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-list
    FACET_PKG xo_printable2
#    REPR List
    INPUT idl/IPrintable_DList.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-list
    FACET_PKG xo_alloc2
#    REPR List
    INPUT idl/IGCObject_DList.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-boolean
    FACET_PKG xo_printable2
#    REPR Boolean
    INPUT idl/IPrintable_DBoolean.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-boolean
    FACET_PKG xo_alloc2
#    REPR Boolean
    INPUT idl/IGCObject_DBoolean.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-float
    FACET_PKG xo_printable2
#    REPR Float
    INPUT idl/IPrintable_DFloat.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-float
    FACET_PKG xo_alloc2
#    REPR Float
    INPUT idl/IGCObject_DFloat.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-integer
    FACET_PKG xo_printable2
#    REPR Integer
    INPUT idl/IPrintable_DInteger.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-integer
    FACET_PKG xo_alloc2
#    REPR Integer
    INPUT idl/IGCObject_DInteger.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-sequence-array
    FACET_PKG xo_object2
#    REPR Array
    INPUT idl/ISequence_DArray.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-array
    FACET_PKG xo_printable2
#    REPR Array
    INPUT idl/IPrintable_DArray.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-array
    FACET_PKG xo_alloc2
#    REPR Array
    INPUT idl/IGCObject_DArray.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-dictionary
    FACET_PKG xo_printable2
#    REPR Dictionary
    INPUT idl/IPrintable_DDictionary.json5
 )

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-dictionary
    FACET_PKG xo_alloc2
#    REPR Dictionary
    INPUT idl/IGCObject_DDictionary.json5
)

# ----------------------------------------------------------------

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-printable-runtimeerror
    FACET_PKG xo_printable2
#    REPR RuntimeError
    INPUT idl/IPrintable_DRuntimeError.json5
 )

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-object2-facetimpl-gcobject-runtimeerror
    FACET_PKG xo_alloc2
#    REPR RuntimeError
    INPUT idl/IGCObject_DRuntimeError.json5
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-object2-genfacet-all)

# ----------------------------------------------------------------

# must complete definition of expression lib before configuring examples
add_subdirectory(src/object2)
add_subdirectory(utest)

install(DIRECTORY idl/
    DESTINATION share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN "*.json5")

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end CMakeLists.txt
