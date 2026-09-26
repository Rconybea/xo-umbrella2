# xo-reactor2websock/src/reactor2websock/CMakeLists.txt

set(SELF_LIB reactor2websock)
set(SELF_SRCS ReactorWebsocketSink.cpp reactor_endpoints.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# PUBLIC: ReactorWebsocketSink IS a reactor::AbstractSink, and the endpoint
# builders take reactor sources and event stores
xo_dependency(${SELF_LIB} reactor)
# PUBLIC: ReactorWebsocketSink wraps a web::WebsocketSink
xo_dependency(${SELF_LIB} websock)
# PUBLIC: the endpoint builders return Stream/HttpEndpointDescr
xo_dependency(${SELF_LIB} webutil)

# end CMakeLists.txt
