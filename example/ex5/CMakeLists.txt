# xo-unit/example/ex5/CMakeLists.txt

set(SELF_EXE xo_unit_ex5)
set(SELF_SRCS ex5.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

# ----------------------------------------------------------------
# dependencies..

xo_self_dependency(${SELF_EXE} xo_unit)
xo_dependency(${SELF_EXE} reflect)

# end CMakeLists.txt
