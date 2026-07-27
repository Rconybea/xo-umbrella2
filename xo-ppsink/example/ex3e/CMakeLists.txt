# NOTE: need target names to be globally unique within the xo umbrella

set(SELF_EXE ppsink_ex3e)
set(SELF_SRCS ex3e.cpp)

if (XO_ENABLE_EXAMPLES)
    add_executable(${SELF_EXE} ${SELF_SRCS})
    xo_include_options2(${SELF_EXE})
    xo_self_dependency(${SELF_EXE} xo_ppsink)
endif()
