# alloc2/CMakeLists.txt

set(SELF_LIB xo_alloc2)
set(SELF_SRCS

    init_alloc2.cpp
    SetupAlloc2.cpp

    CollectorTypeRegistry.cpp
    GCObjectConversion.cpp

    facet/ICollector_Any.cpp

    IGCObject_Any.cpp
    facet/IGCObjectVisitor_Any.cpp

    AAllocator.cpp
    IAllocator_Any.cpp
    IAllocator_DArena.cpp

    IAllocIterator_Any.cpp
    IAllocIterator_DArenaIterator.cpp

    IResourceVisitor_Any.cpp
    )

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must also appear in cmake/xo_alloc2Config.cmake.in
xo_dependency(${SELF_LIB} xo_arena)
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} subsys)
xo_dependency(${SELF_LIB} indentlog)
