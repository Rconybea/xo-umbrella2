# reflect/CMakeLists.txt

set(SELF_LIB reflect)
set(SELF_SRCS
    TypeDescr.cpp TypeDescrExtra.cpp TaggedRcptr.cpp
    atomic/AtomicTdx.cpp
    pointer/PointerTdx.cpp
    vector/VectorTdx.cpp
    struct/StructTdx.cpp struct/StructMember.cpp
    function/FunctionTdx.cpp
    init_reflect.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} refcnt)
xo_dependency(${SELF_LIB} indentlog)
xo_dependency(${SELF_LIB} subsys)
#xo_boost_dependency(${SELF_LIB})

# end CMakeLists.txt
