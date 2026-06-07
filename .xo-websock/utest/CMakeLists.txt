# build unittest websock/utest

set(SELF_EXE utest.websock)
set(SELF_SRCS websock_utest_main.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

## note: can't add this yet,  because test not automated.
##       requires manual interaction from browser
##
#add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
#target_code_coverage(${SELF_EXE} AUTO ALL)

# copy static {.html, .js, .svg} files to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/mount-origin/"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/mount-origin")

# ----------------------------------------------------------------
# internal dependency (on this codebase)

xo_self_dependency(${SELF_EXE} websock)

# ----------------------------------------------------------------
# external dependencies

target_link_libraries(${SELF_EXE} PUBLIC websock)
# Need to port option, volfit before we can build this test here
target_link_libraries(${SELF_EXE} PUBLIC option)
target_link_libraries(${SELF_EXE} PUBLIC volfit)
#target_link_libraries(utest.option PUBLIC logutil)


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
