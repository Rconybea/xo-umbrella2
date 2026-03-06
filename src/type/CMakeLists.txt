# xo-type/src/type/CMakeLists.txt

set(SELF_LIB xo_type)
set(SELF_SRCS
    init_type.cpp
    type_register_facets.cpp
    type_register_types.cpp
    Metatype.cpp
    DAtomicType.cpp
    IType_DAtomicType.cpp
    IGCObject_DAtomicType.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-type/cmake/xo_typeConfig.cmake.in

xo_dependency(${SELF_LIB} xo_alloc2)
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} subsys)

# end src/type/CMakeLists.txt
