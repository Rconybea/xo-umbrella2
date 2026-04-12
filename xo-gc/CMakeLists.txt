# xo-gc/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo_gc VERSION 0.1)

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
xo_add_genfacetimpl(
    TARGET xo-gc-facetimpl-collector-x1collector
    FACET_PKG xo_alloc2
    INPUT idl/ICollector_DX1Collector.json5
)

# note: manual target; generated code committed to git
xo_add_genfacetimpl(
    TARGET xo-gc-facetimpl-gcobjectvisitor-gcobjectstorevisitor
    FACET_PKG xo_alloc2
    INPUT idl/IGCObjectVisitor_DGCObjectStoreVisitor.json5
)

# ----------------------------------------------------------------

xo_add_genfacet_all(xo-gc-genfacet-all)

# ----------------------------------------------------------------

# must complete definition of expression lib before configuring examples
add_subdirectory(src/gc)
add_subdirectory(utest)

install(DIRECTORY idl/
    DESTINATION share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN "*.json5")

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
add_subdirectory(docs)

# end CMakeLists.txt
