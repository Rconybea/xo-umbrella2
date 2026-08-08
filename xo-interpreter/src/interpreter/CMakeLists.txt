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
# TRANSITIONAL.  xo-interpreter has always used legacy xo-indentlog directly --
# VirtualSchematikaMachine.hpp and Schematika.hpp include it, so this is PUBLIC
# -- but inherited it from xo-expression, which is about to migrate to
# xo-ppsink and stop propagating it.  Declared explicitly here so that
# migration does not break this subsystem; retire when xo-interpreter itself
# moves to xo-ppsink.
xo_dependency(${SELF_LIB} indentlog)
xo_external_target_dependency(${SELF_LIB} replxx replxx::replxx)
target_link_libraries(${SELF_LIB} PUBLIC Threads::Threads)
xo_headeronly_dependency(${SELF_LIB} subsys)
