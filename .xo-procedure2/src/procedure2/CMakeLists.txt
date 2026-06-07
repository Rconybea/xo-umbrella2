# xo-procedure2/src/CMakeLists.txt

set(SELF_LIB xo_procedure2)
set(SELF_SRCS
    init_procedure2.cpp
    init_primitives.cpp
    SetupProcedure2.cpp
    ObjectPrimitives.cpp
    GcPrimitives.cpp
    PrimitiveRegistry.cpp
    DPrimitive.cpp
    DSimpleRcx.cpp
    facet/IRuntimeContext_Any.cpp
    facet/IRuntimeContext_DSimpleRcx.cpp
    facet/IProcedure_Any.cpp
    facet/IGCObject_DPrimitive_gco_0.cpp
    facet/IProcedure_DPrimitive_gco_0.cpp
    facet/IPrintable_DPrimitive_gco_0.cpp
    facet/IGCObject_DPrimitive_gco_1_gco.cpp
    facet/IProcedure_DPrimitive_gco_1_gco.cpp
    facet/IPrintable_DPrimitive_gco_1_gco.cpp
    facet/IGCObject_DPrimitive_gco_2_gco_gco.cpp
    facet/IProcedure_DPrimitive_gco_2_gco_gco.cpp
    facet/IPrintable_DPrimitive_gco_2_gco_gco.cpp
    facet/IGCObject_DPrimitive_gco_2_dict_string.cpp
    facet/IProcedure_DPrimitive_gco_2_dict_string.cpp
    facet/IPrintable_DPrimitive_gco_2_dict_string.cpp
    facet/IGCObject_DPrimitive_gco_3_dict_string_gco.cpp
    facet/IProcedure_DPrimitive_gco_3_dict_string_gco.cpp
    facet/IPrintable_DPrimitive_gco_3_dict_string_gco.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/procedure2)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-procedure2/cmake/xo_procedure2Config.cmake.in

xo_dependency(${SELF_LIB} xo_type)
xo_dependency(${SELF_LIB} xo_object2)
xo_dependency(${SELF_LIB} subsys)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end src/CMakeLists.txt
