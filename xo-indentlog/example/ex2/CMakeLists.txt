# NOTE: need target names to be globally unique within the xo umbrella

if (XO_ENABLE_EXAMPLES)
    add_executable(indentlog_ex2 ex2.cpp)
    xo_include_options2(indentlog_ex2)
    xo_self_dependency(indentlog_ex2 indentlog)
endif()
