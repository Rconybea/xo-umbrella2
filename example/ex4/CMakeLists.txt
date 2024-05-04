# xo-unit/example/ex4/CMakeLists.txt

set(SELF_EXE xo_unit_ex4)
set(SELF_SRCS ex4.cpp)

xo_add_executable(${SELF_EXE} ${SELF_SRCS})

# ----------------------------------------------------------------
# dependencies..

xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
xo_dependency(${SELF_EXE} xo_flatstring)

# end CMakeLists.txt
