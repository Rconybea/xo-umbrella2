# xo-websock/utest/CMakeLists.txt

# ----------------------------------------------------------------
# unit tests

set(SELF_EXE utest.websock)
set(SELF_SRCS
    websock_unit_main.cpp
    WebsockUtestAppcx.cpp
    WsSessionRouter.test.cpp
    UrlRouter.test.cpp
    WsSessionTable.test.cpp
    WsSessionSender.test.cpp
    Webserver.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

    xo_self_dependency(${SELF_EXE} websock)
    # websock_unit_main.cpp builds an Indentlog2Appcx: PrintJson and the
    # sink's scope logging run through indentlog2
    xo_dependency(${SELF_EXE} xo_indentlog2)
    # UtestAppStart/UtestListener: shared utest main
    xo_dependency(${SELF_EXE} xo_testutil)
    xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)
endif()

# ----------------------------------------------------------------
# live tests: a started Webserver and a real websocket client.
# Separate from utest.websock, which stays socket-free.
# See .xo-backlog/xo-websock/issues/09.

set(LIVE_EXE utest.websock.live)
set(LIVE_SRCS
    websock_unit_main.cpp
    WebsockUtestAppcx.cpp
    WsTestClient.cpp
    WebserverLive.test.cpp)

if (ENABLE_TESTING)
    xo_add_utest_executable(${LIVE_EXE} ${LIVE_SRCS})

    xo_self_dependency(${LIVE_EXE} websock)
    xo_dependency(${LIVE_EXE} xo_indentlog2)
    xo_dependency(${LIVE_EXE} xo_testutil)
    xo_external_target_dependency(${LIVE_EXE} Catch2 Catch2::Catch2)

    # WsTestClient is a libwebsockets client in its own right.  websock links
    # libwebsockets PRIVATE (see src/websock/CMakeLists.txt), so link it here.
    find_package(Libwebsockets CONFIG REQUIRED)
    target_link_libraries(${LIVE_EXE} PRIVATE websockets_shared)
    # live-server-snapshot-matches-golden reads (and, with XO_UPDATE_GOLDEN,
    # rewrites) utest/golden/ in the source tree
    target_compile_definitions(${LIVE_EXE} PRIVATE
        XO_WEBSOCK_UTEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# ----------------------------------------------------------------
# kalman-era browser demo -- NOT built.
#
# This directory used to hold only a manual browser demo (a websocket + d3 view
# of a live Kalman filter), which cannot build: it needs the `option' and
# `volfit' subsystems, which do not exist in this tree.  Whether to delete it
# or port it is .xo-backlog/xo-websock/issues/01, still undecided.
#
# Real unit tests arrived here on 2026-09-26 (.xo-backlog/xo-websock/issues/04).
# To leave issue 01's decision untouched, the demo's files stay where they are
# and its build stanza is kept verbatim below, disabled.  That is why the unit
# tests' main is websock_unit_main.cpp: websock_utest_main.cpp is the demo's.
#
if(FALSE)
set(SELF_EXE utest.websock)
set(SELF_SRCS websock_utest_main.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

## note: can't add this yet,  because test not automated.
##       requires manual interaction from browser
##
#add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
#target_code_coverage(${SELF_EXE} AUTO ALL)

# copy static {.html, .js, .svg} files to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/mount-origin/"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/mount-origin")

# ----------------------------------------------------------------
# internal dependency (on this codebase)

xo_self_dependency(${SELF_EXE} websock)

# ----------------------------------------------------------------
# external dependencies

target_link_libraries(${SELF_EXE} PUBLIC websock)
# Need to port option, volfit before we can build this test here
target_link_libraries(${SELF_EXE} PUBLIC option)
target_link_libraries(${SELF_EXE} PUBLIC volfit)
#target_link_libraries(utest.option PUBLIC logutil)


# should be getting this via xo_include_options2()

## ----------------------------------------------------------------
## make standard directories for std:: includes explicit
## so that
## (1) they appear in compile_commands.json.
## (2) clangd (run from emacs lsp-mode) can find them
##
#if(CMAKE_EXPORT_COMPILE_COMMANDS)
#  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
#      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
#endif()
endif()

# end CMakeLists.txt
