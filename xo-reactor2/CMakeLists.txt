# xo-reactor2/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_reactor2 VERSION 1.0)
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
    TARGET xo-reactor2-facet-abstracteventprocessor
    FACET AbstractEventProcessor
    INPUT idl/AbstractEventProcessor.json5
    OUTPUT_HPP_DIR include/xo/reactor2
    OUTPUT_IMPL_SUBDIR detail
)

xo_add_genfacet(
    TARGET xo-reactor2-facet-eventsink
    FACET EventSink
    INPUT idl/EventSink.json5
    OUTPUT_HPP_DIR include/xo/reactor2
    OUTPUT_IMPL_SUBDIR detail
)

xo_add_genfacet(
    TARGET xo-reactor2-facet-eventsource
    FACET EventSource
    INPUT idl/EventSource.json5
    OUTPUT_HPP_DIR include/xo/reactor2
    OUTPUT_IMPL_SUBDIR detail
)

# ----------------------------------------------------------------

# note: manual target; generated code comitted to git
xo_add_genfacetimpl(
    TARGET xo-reactor2-facetimpl-gcobject-objectevent
    FACET_PKG xo_alloc2
    INPUT idl/IGCObject_DObjectEvent.json5
    )

# note: manual target; generated code comitted to git
xo_add_genfacetimpl(
    TARGET xo-reactor2-facetimpl-printable-objectevent
    FACET_PKG xo_printable2
    INPUT idl/IPrintable_DObjectEvent.json5
    )

# ----------------------------------------------------------------

install(DIRECTORY idl/
    DESTINATION share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN "*.json5")

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-reactor2-genfacet-all)

# ----------------------------------------------------------------
# output targets

add_subdirectory(src/reactor2)
add_subdirectory(utest)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# end CMakeLists.txt
