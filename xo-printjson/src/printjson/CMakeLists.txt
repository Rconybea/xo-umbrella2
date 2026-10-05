# xo-printjson/src/printjson/CMakeLists.txt

set(SELF_LIB printjson)
set(SELF_SRCS
    PrintJsonAppcx.cpp
    PrintJsonSingleton.cpp
    PrintJson.cpp
    JsonPrintState.cpp
    JsonMembers.cpp
    type_keys.cpp
    init_printjson.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# dependencies: ...

xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} xo_reflectable2)
xo_dependency(${SELF_LIB} xo_facet)
xo_dependency(${SELF_LIB} xo_arena)
# PrintJson.cpp uses xo::pp::{xtag,iso8601}; no public header needs it.
xo_dependency(${SELF_LIB} xo_ppsink)

# PrintJson.cpp: abort when a json printer calls a PrintJson entry point
# instead of JsonPrintState::print (.xo-backlog/xo-printjson/issues/02).  On for now;
# to be turned off later
target_compile_definitions(${SELF_LIB} PRIVATE XO_PRINTJSON_REENTRY_CHECK)

# end CMakeLists.txt
