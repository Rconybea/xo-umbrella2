# xo-ratio/example/ex1/CMakeLists.txt

set(SELF_EXE ratio_ex1)
set(SELF_SRCS ex1.cpp)

if (XO_ENABLE_EXAMPLES)
    add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_include_options2(${SELF_EXE})
    xo_self_dependency(${SELF_EXE} xo_ratio)
    # xo_dependency (not headeronly): xo-ppsink is a compiled library,
    # where xo-indentlog was header-only.
    xo_dependency(${SELF_EXE} xo_ppsink)
endif()

# end CMakeLists.txt
