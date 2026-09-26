# xo-websock/CMakeLists.txt

set(SELF_LIB websock)
set(SELF_SRCS EndpointUtil.cpp DynamicEndpoint.cpp WebsockUtil.cpp WebsocketSink.cpp Webserver.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} printjson)
xo_dependency(${SELF_LIB} xo_ppsink)
xo_dependency(${SELF_LIB} webutil)

# libwebsockets: headers PUBLIC, linkage PRIVATE.
#
# 1. Webserver.hpp and WebsockUtil.hpp include <libwebsockets.h>, so a consumer
#    compiling against xo-websock needs libwebsockets' include directories,
#    and transitively, openssl headers.
# 2. Such a consume does not need to link libwebsockets, since xo-websock
#    takes care of that.
# 3. To make the distinction between 1 and 2, we need websockets_shared
#    linke privately, but include dirs attached publically.
#
# Replaces xo_external_target_dependency(), which can only link PUBLIC.
# See LibwebsocketsTargets-release.cmake for available targets.
find_package(Libwebsockets CONFIG REQUIRED)
target_link_libraries(${SELF_LIB} PRIVATE websockets_shared)
target_include_directories(${SELF_LIB} PUBLIC
    $<TARGET_PROPERTY:websockets_shared,INTERFACE_INCLUDE_DIRECTORIES>)
# see jsoncpp-namespaced-targets.cmake (maybe?) for available targets
xo_external_target_dependency(${SELF_LIB} jsoncpp jsoncpp_lib)

# ----------------------------------------------------------------
# Drop shared libraries this library does not actually use.
#
# libwebsockets' exported cmake target lists openssl
# (libssl.so, libcrypto.so, by full path) in its PUBLIC link interface,
# although openssl is actually a private dependency of libwebsockets.
#
# This triggers unwanted DT_NEEDED entries in libwebsock.
# Suppress them on linux with --as-needed.
# Not needed for OSX; dynamic loader uses absolute paths.
#
if(NOT APPLE)
    target_link_options(${SELF_LIB} PRIVATE "LINKER:--as-needed")
endif()
