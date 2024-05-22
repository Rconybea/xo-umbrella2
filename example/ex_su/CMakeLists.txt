# xo-unit/example/su/CMakeLists.txt

set(SELF_EXE xo_unit_ex_su)
set(SELF_SRCS ex_su.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
    xo_dependency(${SELF_EXE} xo_flatstring)
endif()

# end CMakeLists.txt
