# build unittest reactor/unittest'

set(SELF_EXE utest.reactor)
set(SELF_SRCS Sink.test.cpp PollingReactor.test.cpp reactor_utest_main.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# internal dependency (on this codebase)

xo_self_dependency(${SELF_EXE} reactor)

# ----------------------------------------------------------------
# external dependencies

xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

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

# end CMakeLists.txt
