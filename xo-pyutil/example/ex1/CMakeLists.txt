# xo-pyutil/example/ex1/CMakeLists.txt

set(SELF_LIB xo_pyutilexample)
set(SELF_SRCS pyex1.cpp)

if (XO_ENABLE_EXAMPLES)
    xo_pybind11_library(${SELF_LIB} ${PROJECT_NAME}Targets ${SELF_SRCS})
    xo_pybind11_dependency(${SELF_LIB} refcnt)
endif()

# end CMakeLists.txt
