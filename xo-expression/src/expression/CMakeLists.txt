# expression/CMakeLists.txt

set(SELF_LIB xo_expression)
set(SELF_SRCS
    GeneralizedExpression.cpp
    Expression.cpp
    DefineExpr.cpp
    AssignExpr.cpp
    Apply.cpp
    Lambda.cpp
    Variable.cpp
    IfExpr.cpp
    Sequence.cpp
    GlobalSymtab.cpp
    LocalSymtab.cpp
    ConvertExpr.cpp
    PrimitiveExpr.cpp
    typeinf/type_ref.cpp
    typeinf/type_unifier.cpp
    typeinf/TypeBlueprint.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} xo_flatstring)
# public headers name xo::pp::PpSink (GeneralizedExpression::pretty)
xo_dependency(${SELF_LIB} xo_ppsink)
#xo_dependency(${SELF_LIB} subsys)

# end CMakeLists.txt
