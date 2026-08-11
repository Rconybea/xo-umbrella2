# xo-hashable2/src/hashable2/CMakeLists.txt

set(SELF_LIB xo_hashable2)
set(SELF_SRCS
    IHashable_Any.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# input dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-hashable2/cmake/xo_hashable2Config.cmake.in

xo_dependency(${SELF_LIB} xo_facet)
#xo_dependency(${SELF_LIB} xo_indentlog)

# end src/hashable2/CMakeLists.txt
