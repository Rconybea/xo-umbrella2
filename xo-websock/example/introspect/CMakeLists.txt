# xo-websock/example/introspect/CMakeLists.txt
#
# A webserver that shows its own state in the browser.
# See .xo-backlog/xo-websock/issues/10.

set(SELF_EXE websock_ex_introspect)
set(SELF_SRCS introspect.cpp)

if (XO_ENABLE_EXAMPLES)
    add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_include_options2(${SELF_EXE})

    xo_self_dependency(${SELF_EXE} websock)
    xo_dependency(${SELF_EXE} xo_indentlog2)

    # the server serves ./mount-origin, relative to its working directory:
    # keep a copy beside the executable, refreshed on every build
    add_custom_command(TARGET ${SELF_EXE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/mount-origin
                ${CMAKE_CURRENT_BINARY_DIR}/mount-origin)
endif()

# end CMakeLists.txt
