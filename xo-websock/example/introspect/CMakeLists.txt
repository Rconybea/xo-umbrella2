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

    # the page, copied beside the executable (introspect.cpp serves it from
    # there).  One rule per file, each depending on its source, so editing
    # only the page still refreshes the copy -- a POST_BUILD step would run
    # only when the executable relinks.  CONFIGURE_DEPENDS: a new page file
    # is picked up without re-running cmake by hand.
    file(GLOB_RECURSE PAGE_SRCS CONFIGURE_DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/mount-origin/*)

    set(PAGE_COPIES)
    foreach(src ${PAGE_SRCS})
        file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${src})
        set(dst ${CMAKE_CURRENT_BINARY_DIR}/${rel})
        add_custom_command(OUTPUT ${dst}
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
                           DEPENDS ${src})
        list(APPEND PAGE_COPIES ${dst})
    endforeach()

    add_custom_target(${SELF_EXE}_page ALL DEPENDS ${PAGE_COPIES})
    add_dependencies(${SELF_EXE} ${SELF_EXE}_page)
endif()

# end CMakeLists.txt
