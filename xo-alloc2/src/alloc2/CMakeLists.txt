# alloc2/CMakeLists.txt

set(SELF_LIB xo_alloc2)
set(SELF_SRCS

    AllocInfo.cpp
    cmpresult.cpp

    AAllocator.cpp
    DArena.cpp
    IAllocator_Any.cpp
    IAllocator_DArena.cpp

    IAllocIterator_Any.cpp
    DArenaIterator.cpp
    IAllocIterator_DArenaIterator.cpp

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
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} indentlog)
