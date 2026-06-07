# xo-unit/example/ex1/CMakeLists.txt

set(SELF_EXE xo_unit_ex1)
set(SELF_SRCS ex1.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
    xo_headeronly_dependency(${SELF_EXE} xo_ratio)
    # bug -- headeronly dependencies not getting propagated
    xo_headeronly_dependency(${SELF_EXE} xo_flatstring)
    xo_headeronly_dependency(${SELF_EXE} indentlog)
endif()

# end CMakeLists.txt
