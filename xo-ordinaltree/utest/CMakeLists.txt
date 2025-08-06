# ordinaltree/utest/CMakeLists.txt

# note: tests in this directory use Catch2-provided main
set(SELF_EXE utest.tree)
set(SELF_SOURCE_FILES tree_utest_main.cpp redblacktree.cpp bplustree.cpp)

add_executable(${SELF_EXE} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
# need coverage on unit test b/c header-only libraries
#target_code_coverage(${SELF_EXE} EXCLUDE */nix/store* utest/*)
#target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# internal dependencies: refcnt, ...

xo_dependency(${SELF_EXE} refcnt)
xo_dependency(${SELF_EXE} indentlog)
xo_dependency(${SELF_EXE} randomgen)

# ----------------------------------------------------------------
# 3rd part dependency: catch2:

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end ordinaltree/utest/CMakeLists.txt
