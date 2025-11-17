# interpreter/CMakeLists.txt

set(SELF_LIB xo_interpreter)
set(SELF_SRCS
    init_interpreter.cpp
    StackFrame.cpp
    VirtualSchematikaMachine.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_alloc)
#xo_dependency(${SELF_LIB} xo_reader)
xo_headeronly_dependency(${SELF_LIB} subsys)
