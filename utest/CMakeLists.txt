# build unittest 'refcnt/utest/utest.refcnt

set(SELF_EXECUTABLE_NAME utest.refcnt)
# These tests can use the Catch2-provided main
set(SELF_SOURCE_FILES intrusive_ptr.test.cpp refcnt_utest_main.cpp)

add_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXECUTABLE_NAME})

add_test(NAME ${SELF_EXECUTABLE_NAME} COMMAND ${SELF_EXECUTABLE_NAME})
target_code_coverage(${SELF_EXECUTABLE_NAME} AUTO ALL)

#target_link_libraries(${SELF_EXECUTABLE_NAME} PRIVATE Catch2::Catch2WithMain)

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
#target_include_directories(${SELF_EXECUTABLE_NAME} PUBLIC
#                           ${PROJECT_SOURCE_DIR}
#                           ${PROJECT_BINARY_DIR})

# ----------------------------------------------------------------
# internal dependencies: refcnt, ...

xo_self_dependency(${SELF_EXECUTABLE_NAME} refcnt)
#target_link_libraries(${SELF_EXECUTABLE_NAME} PUBLIC refcnt)

# ----------------------------------------------------------------
# 3rd party dependency: catch2:

xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# need this so that catch2/include appears in compile_commands.json,
# on which lsp integration relies.
#
# See also /nix/store/*-catch2-*/lib/cmake/Catch2/ParseAndAddCatchTests.cmake;
# commands here derived from ^ .cmake file
#
#find_path(CATCH_INCLUDE_DIR "catch2/catch.hpp")
#target_include_directories(${SELF_EXECUTABLE_NAME} PUBLIC ${CATCH_INCLUDE_DIR})

# ----------------------------------------------------------------
# make standard directories for std:: includes explicit
# so that
# (1) they appear in compile_commands.json.
# (2) clangd (run from emacs lsp-mode) can find them
#
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# end CMakeLists.txt
