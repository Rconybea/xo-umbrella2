# xo-stringtable2/src/stringtable2/CMakeLists.txt

set(SELF_LIB xo_stringtable2)
set(SELF_SRCS
    init_stringtable2.cpp
    SetupStringtable2.cpp

    Stringtable2Appcx.cpp

    StringTable.cpp

    DString.cpp
    IGCObject_DString.cpp
    IPrintable_DString.cpp
    IReflectable_DString.cpp

    DUniqueString.cpp
    IGCObject_DUniqueString.cpp
    IPrintable_DUniqueString.cpp

)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/stringtable2)

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: no parallel list to keep in step -- xo_stringtable2Config.cmake.in uses
#       the generated @XO_FIND_DEPENDENCY_BLOCK@, so find_dependency() calls
#       come from the lines below.  (xo-object2 still had the hand-maintained
#       kind when it took its xo_reflectable2 dependency, and the exported
#       config went stale; see .xo-backlog/generated-find-dependency/.)

xo_dependency(${SELF_LIB} xo_alloc2)
xo_dependency(${SELF_LIB} xo_printable2)
xo_dependency(${SELF_LIB} xo_facet)
# AReflectable for DString, + the TaggedPtr it hands back
xo_dependency(${SELF_LIB} xo_reflectable2)
xo_dependency(${SELF_LIB} reflect)
# SetupStringtable2::provide_json_printers
xo_dependency(${SELF_LIB} printjson)
xo_dependency(${SELF_LIB} subsys)

# end src/stringtable2/CMakeLists.txt
