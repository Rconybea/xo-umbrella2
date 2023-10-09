# build unittest printjson/utest

set(SELF_EXE utest.printjson)
set(SELF_SRCS printjson_utest_main.cpp PrintJson.test.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# generic project dependency

## PROJECT_SOURCE_DIR:
##   so we can for example write
##     #include "indentlog/scope.hpp"
##   from anywhere in the project
## PROJECT_BINARY_DIR:
##   since version file will be in build directory,  need that directory
##   to also be included in compiler's include path
##
#target_include_directories(${SELF_EXE} PUBLIC
#                           ${PROJECT_SOURCE_DIR}
#               ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# dependencies on this codebase

xo_self_dependency(${SELF_EXE} printjson)

# ----------------------------------------------------------------
# dependencies on other codebases

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# need this so that catch2/include appears in compile_commands.json,
# on which lsp integration relies.
#
# See also /nix/store/*-catch2-*/lib/cmake/Catch2/ParseAndAddCatchTests.cmake;
# commands here derived from ^ .cmake file
#
#find_path(CATCH_INCLUDE_DIR "catch2/catch.hpp")
#target_include_directories(${SELF_EXE} PUBLIC ${CATCH_INCLUDE_DIR})

# supplied from xo_include_options2()
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

# end CMakeLists.txt
