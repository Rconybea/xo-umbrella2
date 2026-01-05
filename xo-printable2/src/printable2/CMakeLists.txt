# printable2/CMakeLists.txt

set(SELF_LIB xo_printable2)
set(SELF_SRCS
    IPrintable_Any.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
# note: deps here must coord with cmake/xo_printable2Config.cmake.in
xo_dependency(${SELF_LIB} indentlog)
xo_dependency(${SELF_LIB} xo_facet)
