# object2/CMakeLists.txt

set(SELF_LIB xo_object2)
set(SELF_SRCS
    IGCObject_DFloat.cpp
    IGCObject_DInteger.cpp
    IGCObject_DList.cpp
    ISequence_Any.cpp
    ISequence_DList.cpp
    DList.cpp
    DFloat.cpp
    object2_register_types.cpp
    )

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must also appear in cmake/xo_alloc2Config.cmake.in
xo_dependency(${SELF_LIB} xo_gc)
#xo_dependency(${SELF_LIB} indentlog)
#add_dependencies(${SELF_LIB} xo-object2-facetimpl-sequence-list)
