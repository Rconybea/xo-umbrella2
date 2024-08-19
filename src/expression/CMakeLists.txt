# expression/CMakeLists.txt

set(SELF_LIB xo_expression)
set(SELF_SRCS
    Expression.cpp
    DefineExpr.cpp
    Apply.cpp
    Lambda.cpp
    Variable.cpp
    IfExpr.cpp
    LocalEnv.cpp
    ConvertExpr.cpp
    Primitive.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} reflect)
#xo_dependency(${SELF_LIB} indentlog)
#xo_dependency(${SELF_LIB} subsys)

# end CMakeLists.txt
