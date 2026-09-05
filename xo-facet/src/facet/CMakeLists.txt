# facet/CMakeLists.txt

set(SELF_LIB xo_facet)
set(SELF_SRCS
    IAllocator_Any.cpp
    IAllocIterator_Any.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# note: deps here must coordinate with cmake/xo_facetConfig.cmake.in
xo_dependency(${SELF_LIB} xo_arena)
xo_dependency(${SELF_LIB} xo_reflectutil)
xo_dependency(${SELF_LIB} xo_indentlog2)

# end CMakeLists.txt
