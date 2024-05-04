# xo-unit/example/ex6/CMakeLists.txt

set(SELF_EXE xo_unit_ex6)
set(SELF_SRCS ex6.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

# ----------------------------------------------------------------
# dependencies..

xo_self_headeronly_dependency(${SELF_EXE} xo_unit)
xo_dependency(${SELF_EXE} xo_flatstring)

# end CMakeLists.txt
