# xo-type/src/type/CMakeLists.txt

set(SELF_LIB xo_type)
set(SELF_SRCS
    init_type.cpp
    SetupType.cpp
    TypeOps.cpp
    Metatype.cpp
    DAtomicType.cpp
    DListType.cpp
    DArrayType.cpp
    DFunctionType.cpp
    DTypeVarRef.cpp
    IType_Any.cpp
    IType_DAtomicType.cpp
    IType_DListType.cpp
    IType_DArrayType.cpp
    IType_DFunctionType.cpp
    IType_DTypeVarRef.cpp
    IGCObject_DAtomicType.cpp
    IGCObject_DListType.cpp
    IGCObject_DArrayType.cpp
    IGCObject_DFunctionType.cpp
    IGCObject_DTypeVarRef.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-type/cmake/xo_typeConfig.cmake.in

xo_dependency(${SELF_LIB} xo_object2)
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} xo_alloc2)
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} subsys)

# end src/type/CMakeLists.txt
