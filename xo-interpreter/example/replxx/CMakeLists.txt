# xo-interpreter/example/replxx/CMakeLists.txt

set(SELF_EXE xo_interpreter_replxx)
set(SELF_SRCS replxx.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_dependency(${SELF_EXE} xo_interpreter)
    # TODO: consider promoting to regular app
    xo_dependency(${SELF_EXE} xo_reader)
    xo_external_target_dependency(${SELF_EXE} replxx replxx::replxx)

    find_package(Threads REQUIRED)
    target_link_libraries(${SELF_EXE} PUBLIC Threads::Threads)
endif()

# end CMakeLists.txt
