# reader/CMakeLists.txt

set(SELF_LIB xo_reader)
set(SELF_SRCS
    parser_result.cpp
    parser.cpp
    parserstatemachine.cpp
    reader.cpp
    exprstate.cpp
    exprstatestack.cpp
    define_xs.cpp
    if_else_xs.cpp
    progress_xs.cpp
    apply_xs.cpp
    paren_xs.cpp
    sequence_xs.cpp
    exprseq_xs.cpp
    expect_expr_xs.cpp
    expect_symbol_xs.cpp
    expect_formal_xs.cpp
    expect_formal_arglist_xs.cpp
    expect_type_xs.cpp
    lambda_xs.cpp
    let1_xs.cpp
    envframestack.cpp
    pretty_parserstatemachine.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_expression)
xo_dependency(${SELF_LIB} xo_tokenizer)

# end CMakeLists.txt
