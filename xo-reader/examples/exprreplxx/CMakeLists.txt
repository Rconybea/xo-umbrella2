# xo-reader/example/exprreplxx/CMakeLists.txt

set(SELF_EXE xo_expression_replxx)
set(SELF_SRCS exprreplxx.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_dependency(${SELF_EXE} xo_reader)
    xo_external_target_dependency(${SELF_EXE} replxx replxx::replxx)

    find_package(Threads REQUIRED)
    target_link_libraries(${SELF_EXE} PUBLIC Threads::Threads)
    #xo_external_target_dependency(${SELF_EXE} Threads Threads::Threads)
endif()

# end CMakeLists.txt
