# object2/CMakeLists.txt

set(SELF_LIB xo_object2)
set(SELF_SRCS
    IGCObject_DFloat.cpp
    IGCObject_DInteger.cpp
    IGCObject_DList.cpp
    IGCObject_DString.cpp
    ISequence_Any.cpp
    ISequence_DList.cpp
    IPrintable_DList.cpp
    IPrintable_DFloat.cpp
    IPrintable_DInteger.cpp
    IPrintable_DString.cpp
    DList.cpp
    DFloat.cpp
    DInteger.cpp
    DString.cpp
    object2_register_types.cpp
    object2_register_facets.cpp
    )

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must also appear in cmake/xo_object2Config.cmake.in
xo_dependency(${SELF_LIB} xo_gc)
xo_dependency(${SELF_LIB} xo_printable2)
xo_dependency(${SELF_LIB} indentlog)
