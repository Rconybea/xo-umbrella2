# build unittest printjson/utest

set(SELF_EXE utest.printjson)
set(SELF_SRCS printjson_utest_main.cpp PrintJson.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})
xo_self_dependency(${SELF_EXE} printjson)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
