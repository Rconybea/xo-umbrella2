# build unittest printjson/utest

set(SELF_EXE utest.printjson)
set(SELF_SRCS printjson_utest_main.cpp PrintJson.test.cpp FopJson.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} printjson)
# test-only: FopJson.test.cpp reflects a faceted object pointer.  The library edge
# printjson -> xo-reflectable2 arrives with the entry point in issues/04.
xo_dependency(${SELF_EXE} xo_reflectable2)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
