# object2/CMakeLists.txt

set(SELF_LIB xo_object2)
set(SELF_SRCS
    init_object2.cpp
    object2_register_types.cpp
    object2_register_facets.cpp

    GCObjectConversion_DFloat.cpp
    GCObjectConversion_DInteger.cpp

    ISequence_Any.cpp

    DArray.cpp
    ISequence_DArray.cpp
    IGCObject_DArray.cpp
    IPrintable_DArray.cpp

    DList.cpp
    ISequence_DList.cpp
    IGCObject_DList.cpp
    IPrintable_DList.cpp

    DFloat.cpp
    IGCObject_DFloat.cpp
    IPrintable_DFloat.cpp

    DInteger.cpp
    IGCObject_DInteger.cpp
    IPrintable_DInteger.cpp

    DBoolean.cpp
    IGCObject_DBoolean.cpp
    IPrintable_DBoolean.cpp

    DString.cpp
    IGCObject_DString.cpp
    IPrintable_DString.cpp

    DRuntimeError.cpp
    IGCObject_DRuntimeError.cpp
    IPrintable_DRuntimeError.cpp
    )

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must also appear in cmake/xo_object2Config.cmake.in
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} xo_gc)
xo_dependency(${SELF_LIB} xo_printable2)
xo_dependency(${SELF_LIB} subsys)
xo_dependency(${SELF_LIB} indentlog)
