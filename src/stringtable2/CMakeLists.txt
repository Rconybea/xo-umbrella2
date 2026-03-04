# xo-stringtable2/src/stringtable2/CMakeLists.txt

set(SELF_LIB xo_stringtable2)
set(SELF_SRCS
    init_stringtable2.cpp
    stringtable2_register_facets.cpp
    stringtable2_register_types.cpp

    DString.cpp
    IGCObject_DString.cpp
    IPrintable_DString.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/stringtable2)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-stringtable2/cmake/xo_stringtable2Config.cmake.in

xo_dependency(${SELF_LIB} xo_alloc2)
xo_dependency(${SELF_LIB} xo_printable2)
xo_dependency(${SELF_LIB} subsys)
xo_dependency(${SELF_LIB} indentlog)

# end src/stringtable2/CMakeLists.txt
