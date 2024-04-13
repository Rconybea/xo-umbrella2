# xo-stringliteral/example/ex1/CMakeLists.txt

set(SELF_EXE xo_stringliteral_ex1)
set(SELF_SRCS ex1.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

# ----------------------------------------------------------------
# dependencies..

xo_self_dependency(${SELF_EXE} xo_stringliteral)
#xo_dependency(${SELF_EXE} reflect)

# end CMakeLists.txt
