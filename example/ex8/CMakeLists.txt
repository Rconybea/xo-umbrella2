# xo-unit/example/ex8/CMakeLists.txt

set(SELF_EXE xo_unit_ex8)
set(SELF_SRCS ex8.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
    xo_dependency(${SELF_EXE} xo_flatstring)
endif()

# end CMakeLists.txt
