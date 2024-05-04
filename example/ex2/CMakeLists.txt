# xo-unit/example/ex2/CMakeLists.txt

set(SELF_EXE xo_unit_ex2)
set(SELF_SRCS ex2.cpp)

xo_add_executable(${SELF_EXE} ${SELF_SRCS})

# ----------------------------------------------------------------
# dependencies..

xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
xo_dependency(${SELF_EXE} xo_flatstring)

# end CMakeLists.txt
