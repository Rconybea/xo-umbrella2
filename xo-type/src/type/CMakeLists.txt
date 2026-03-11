# xo-type/src/type/CMakeLists.txt

set(SELF_LIB xo_type)
set(SELF_SRCS
    init_type.cpp
    type_register_facets.cpp
    type_register_types.cpp
    TypeOps.cpp
    Metatype.cpp
    DAtomicType.cpp
    DListType.cpp
    DArrayType.cpp
    DFunctionType.cpp
    IType_Any.cpp
    IType_DAtomicType.cpp
    IType_DListType.cpp
    IType_DArrayType.cpp
    IType_DFunctionType.cpp
    IGCObject_DAtomicType.cpp
    IGCObject_DListType.cpp
    IGCObject_DArrayType.cpp
    IGCObject_DFunctionType.cpp
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
