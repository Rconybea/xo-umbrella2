# xo-unit/example/ex_qty/CMakeLists.txt

set(SELF_EXE xo_unit_ex_qty)
set(SELF_SRCS ex_qty.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
    xo_dependency(${SELF_EXE} xo_flatstring)
endif()

# end CMakeLists.txt
