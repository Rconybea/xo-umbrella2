# build unittest printjson/utest

set(SELF_EXE utest.printjson)
set(SELF_SRCS printjson_utest_main.cpp PrintJson.test.cpp FopJson.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} printjson)
# UtestAppStart/UtestListener: shared utest main, see printjson_utest_main.cpp
xo_dependency(${SELF_EXE} xo_testutil)
# printjson_utest_main.cpp builds an Indentlog2Appcx directly
xo_dependency(${SELF_EXE} xo_indentlog2)
# test-only: a D-type that is printable but NOT reflectable, for the
# has-not-opted-in case
xo_dependency(${SELF_EXE} xo_printable2)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
