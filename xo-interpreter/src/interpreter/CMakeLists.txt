# interpreter/CMakeLists.txt

set(SELF_LIB xo_interpreter)
set(SELF_SRCS
    init_interpreter.cpp
    StackFrame.cpp
    Schematika.cpp
    VirtualSchematikaMachine.cpp
)

find_package(Threads REQUIRED)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_alloc)
xo_dependency(${SELF_LIB} xo_expression)
xo_dependency(${SELF_LIB} xo_reader)
xo_external_target_dependency(${SELF_LIB} replxx replxx::replxx)
target_link_libraries(${SELF_LIB} PUBLIC Threads::Threads)
xo_headeronly_dependency(${SELF_LIB} subsys)
