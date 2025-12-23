# gc/CMakeLists.txt

set(SELF_LIB xo_gc)
set(SELF_SRCS

    ICollector_Any.cpp
    IGCObject_Any.cpp
    IAllocator_DX1Collector.cpp
    ICollector_DX1Collector.cpp
    IAllocIterator_DX1CollectorIterator.cpp

    DX1Collector.cpp
    DX1CollectorIterator.cpp

    )

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must also appear in cmake/xo_alloc2Config.cmake.in
xo_dependency(${SELF_LIB} xo_alloc2)
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} indentlog)
