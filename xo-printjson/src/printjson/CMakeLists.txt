# xo-printjson/src/printjson/CMakeLists.txt

set(SELF_LIB printjson)
set(SELF_SRCS PrintJson.cpp init_printjson.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# dependencies: indentlog, ...

xo_dependency(${SELF_LIB} reflect)

# end CMakeLists.txt
