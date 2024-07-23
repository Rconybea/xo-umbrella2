# tokenizer/CMakeLists.txt

set(SELF_LIB xo_tokenizer)
set(SELF_SRCS
    tokentype.cpp
    token.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
#xo_dependency(${SELF_LIB} refcnt)
xo_dependency(${SELF_LIB} indentlog)
#xo_dependency(${SELF_LIB} subsys)
#xo_boost_dependency(${SELF_LIB})

# end CMakeLists.txt
