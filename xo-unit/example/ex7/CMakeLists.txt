# xo-unit/example/ex7/CMakeLists.txt

set(SELF_EXE xo_unit_ex7)
set(SELF_SRCS ex7.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
    xo_dependency(${SELF_EXE} xo_flatstring)
endif()

# end CMakeLists.txt
