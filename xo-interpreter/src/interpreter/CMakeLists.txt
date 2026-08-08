# interpreter/CMakeLists.txt

set(SELF_LIB xo_interpreter)
set(SELF_SRCS
    init_interpreter.cpp
    Schematika.cpp
    BuiltinPrimitives.cpp
    LocalEnv.cpp
    GlobalEnv.cpp
    VirtualSchematikaMachine.cpp
    VsmInstr.cpp
    VsmStackFrame.cpp
    ExpressionBoxed.cpp
)

find_package(Threads REQUIRED)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_object)
xo_dependency(${SELF_LIB} xo_expression)
xo_dependency(${SELF_LIB} xo_reader)
# VirtualSchematikaMachine.hpp / Schematika.hpp name xo::pp::log_level in their
# public API, so this is PUBLIC.
xo_dependency(${SELF_LIB} xo_ppsink)
# Schematika.cpp renders expressions with line breaking (PrettySink)
xo_dependency(${SELF_LIB} xo_indentlog2)
xo_external_target_dependency(${SELF_LIB} replxx replxx::replxx)
target_link_libraries(${SELF_LIB} PUBLIC Threads::Threads)
xo_headeronly_dependency(${SELF_LIB} subsys)
