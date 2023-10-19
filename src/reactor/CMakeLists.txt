# xo-reactor/src/reactor/CMakeLists.txt

set(SELF_LIB reactor)
set(SELF_SRCS
    AbstractEventProcessor.cpp AbstractSource.cpp ReactorSource.cpp
    Sink.cpp
    Reactor.cpp PollingReactor.cpp
    init_reactor.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# note: changes to xo_dependency() calls here
#       must coordinate with find_dependency() calls
#       in xo-reactor/cmake/reactorConfig.cmake.in
#
#xo_dependency(${SELF_LIB} xo_ordinaltree)  # for some reason wants to link -lxo_ordinaltree ??
xo_dependency(${SELF_LIB} reflect)
xo_dependency(${SELF_LIB} webutil)
xo_dependency(${SELF_LIB} printjson)
xo_dependency(${SELF_LIB} callback)
