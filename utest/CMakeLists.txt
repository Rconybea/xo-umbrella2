# indentlog unit test

set(SELF_EXECUTABLE_NAME utest.indentlog)
set(SELF_SOURCE_FILES
    fixed.test.cpp quoted.test.cpp vector.test.cpp array.test.cpp timeutil.test.cpp tag.test.cpp
    filename.test.cpp code_location.test.cpp function.test.cpp
    indentlog_utest_main.cpp)

add_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXECUTABLE_NAME})

add_test(NAME ${SELF_EXECUTABLE_NAME} COMMAND ${SELF_EXECUTABLE_NAME})
target_code_coverage(${SELF_EXECUTABLE_NAME} AUTO ALL)

# ----------------------------------------------------------------
# 3rd party dependency: catch2

xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# 1. using target_link_libraries(.. Catch2) here doesn't work;
#    build tries to link to a Catch2 library.
#    perhaps Catch2 target not declared INTERFACE?
# 2. nix build doesn't need this;  dependency on catch2
#    puts include directory in NIX_CFLAGS_COMPILE,  so stuff 'just works'
#

#find_package(Catch2 2 REQUIRED)
#target_link_libraries(${SELF_EXECUTABLE_NAME} Catch2::Catch2)

#set(Catch2_INCLUDES "${Catch2_DIR}/../../../include")
#target_include_directories(${SELF_EXECUTABLE_NAME} PUBLIC ${Catch2_INCLUDES})

#xo_internal_dependency(${SELF_EXECUTABLE_NAME} Catch2)
#target_link_libraries(${SELF_EXECUTABLE_NAME} PUBLIC Catch2)

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
