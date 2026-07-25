# xo-indentlog2/src/indentlog2/CMakeLists.txt

set(SELF_LIB xo_indentlog2)
set(SELF_SRCS
    LogStreambuf.cpp
	LogConfig.cpp
    LogBuffer.cpp
	PpSink.cpp
    PrettySink.cpp
    FlatSink.cpp
	PpState.cpp
    LineState.cpp
	PpTokenStreambuf.cpp
    PpToken.cpp
    PpConfig.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/indentlog2)

# ----------------------------------------------------------------
# dependencies

# NOTE: dependency set here must be kept consistent with
#       xo-indentlog2/cmake/xo_indentlog2Config.cmake.in

xo_dependency(${SELF_LIB} xo_arena)

# end CMakeLists.txt
