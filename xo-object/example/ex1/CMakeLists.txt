# xo-object/example/ex1/CMakeLists.txt

set(SELF_EXE xo_object_ex1)
set(SELF_SRCS ex1.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_self_headeronly_dependency(${SELF_EXE} xo_reflect)
    xo_dependency(${SELF_EXE} xo_flatstring)
endif()

# end CMakeLists.txt
