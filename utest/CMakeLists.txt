# tree/utest/CMakeLists.txt

# note: tests in this directory use Catch2-provided main
set(SELF_EXE utest.tree)
set(SELF_SOURCE_FILES tree_utest_main.cpp redblacktree.cpp bplustree.cpp)

add_executable(${SELF_EXE} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# internal dependencies: refcnt, ...

xo_dependency(${SELF_EXE} refcnt)
xo_dependency(${SELF_EXE} indentlog)
xo_dependency(${SELF_EXE} randomgen)

# ----------------------------------------------------------------
# 3rd part dependency: catch2:

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# need this so that catch2/include appears in compile_commands.json,
# on which lsp integration relies.
#
# See also /nix/store/*-catch2-*/lib/cmake/Catch2/ParseAndAddCatchTests.cmake;
# commands here derived from ^ .cmake file
#

# let's see if xo_external_target_dependency() works for these..
#find_path(CATCH_INCLUDE_DIR "catch2/catch.hpp")
#target_include_directories(${SELF_UTEST_NAME} PUBLIC ${CATCH_INCLUDE_DIR})

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

# end tree/utest/CMakeLists.txt
