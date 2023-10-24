# xo-websock/CMakeLists.txt

set(SELF_LIB websock)
set(SELF_SRCS EndpointUtil.cpp DynamicEndpoint.cpp WebsockUtil.cpp WebsocketSink.cpp Webserver.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# note: changes to xo_dependency() calls here
#       must coordinate with find_dependency() calls in
#       xo-websock/cmake/websockConfig.cmake.in
xo_dependency(${SELF_LIB} reactor)
xo_dependency(${SELF_LIB} webutil)

# see LibwebsocketsTargets-release.cmake for available targets
xo_external_target_dependency(${SELF_LIB} Libwebsockets websockets_shared)
# see jsoncpp-namespaced-targets.cmake (maybe?) for available targets
xo_external_target_dependency(${SELF_LIB} jsoncpp jsoncpp_lib)
