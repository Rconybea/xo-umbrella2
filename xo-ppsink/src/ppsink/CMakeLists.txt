# xo-ppsink/src/ppsink/CMakeLists.txt

set(SELF_LIB xo_ppsink)
set(SELF_SRCS
    PpSink.cpp
    FlatSink.cpp
    LogState.cpp
    scope.cpp
    color.cpp
    function_name.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/ppsink)

# ----------------------------------------------------------------
# dependencies
#
# NOTE: dependency set here must be kept consistent with
#       xo-ppsink/cmake/xo_ppsinkConfig.cmake.in

xo_headeronly_dependency(${SELF_LIB} xo_timeutil)

# end CMakeLists.txt
