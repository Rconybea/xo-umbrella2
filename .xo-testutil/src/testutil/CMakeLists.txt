# testutil/CMakeLists.txt

set(SELF_LIB xo_testutil)
set(SELF_SRCS
    UtestAppStart.cpp
    UtestConfig.cpp
    Utest.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_install_include_tree3(include/xo/testutil)
# note: deps here must also appear in cmake/xo_testutilConfig.cmake.in
xo_dependency(${SELF_LIB} subsys)
xo_dependency(${SELF_LIB} indentlog)
xo_external_target_dependency(${SELF_LIB} Catch2 Catch2::Catch2)
