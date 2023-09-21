# indentlog unit test

set(SELF_EXECUTABLE_NAME utest.indentlog)
set(SELF_SOURCE_FILES fixed.test.cpp quoted.test.cpp vector.test.cpp array.test.cpp timeutil.test.cpp indentlog_utest_main.cpp)

add_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_include_options(${SELF_EXECUTABLE_NAME})

add_test(NAME ${SELF_EXECUTABLE_NAME} COMMAND ${SELF_EXECUTABLE_NAME})
target_code_coverage(${SELF_EXECUTABLE_NAME} AUTO ALL)

# ----------------------------------------------------------------
# 3rd party dependency: catch2

find_package(Catch2 2 REQUIRED)

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
