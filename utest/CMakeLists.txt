# xo-flatstring/utest/CMakeLists.txt

set(SELF_EXE utest.flatstring)
set(SELF_SRCS
    flatstring_utest_main.cpp
    flatstring.test.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
#target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# deps:  logutils, ...

xo_self_headeronly_dependency(${SELF_EXE} xo_flatstring)
xo_dependency(${SELF_EXE} indentlog)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
