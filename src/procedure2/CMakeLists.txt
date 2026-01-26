# xo-procedure2/src/CMakeLists.txt

set(SELF_LIB xo_procedure2)
set(SELF_SRCS
    init_procedure2.cpp
    init_primitives.cpp
    DPrimitive.cpp
    IRuntimeContext_Any.cpp
    IProcedure_Any.cpp
    IGCObject_DPrimitive_gco_2_gco_gco.cpp
    IProcedure_DPrimitive_gco_2_gco_gco.cpp
    # Add source files here, e.g.:
    # procedure2.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/procedure2)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-procedure2/cmake/xo_procedure2Config.cmake.in

xo_dependency(${SELF_LIB} xo_object2)
xo_dependency(${SELF_LIB} xo_gc)
xo_dependency(${SELF_LIB} subsys)
#xo_dependency(${SELF_LIB} xo_indentlog)

xo_export_cmake_config(${PROJECT_NAME} ${PROJECT_VERSION} ${PROJECT_NAME}Targets)

# ----------------------------------------------------------------
# docs targets depend on other library/utest/exec targets above,
# --> must come after them.
#
#add_subdirectory(docs)

# end src/CMakeLists.txt
