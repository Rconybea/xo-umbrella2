# jit/CMakeLists.txt

set(SELF_LIB xo_jit)
set(SELF_SRCS
    LlvmContext.cpp
    IrPipeline.cpp
    MachPipeline.cpp
    intrinsics.cpp
    activation_record.cpp
    type2llvm.cpp
)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})
xo_dependency(${SELF_LIB} xo_expression)
# TRANSITIONAL.  xo-jit uses legacy xo-indentlog in its own sources (MachPipeline,
# type2llvm, activation_record) but inherited it from xo-expression, which is
# about to migrate to xo-ppsink and stop propagating it.  Declared explicitly so
# that migration does not break this subsystem; retire when xo-jit itself moves
# to xo-ppsink.  NB no public header of xo-jit includes indentlog, so this is
# broader than strictly needed -- xo has no PRIVATE dependency form.
xo_dependency(${SELF_LIB} indentlog)

# llvm {16,17} api will not build without some retro work
find_package(LLVM 18.1 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM_DIR=${LLVM_DIR}")
message(STATUS "LLVM_DEFINITIONS=${LLVM_DEFINITIONS}")
message(STATUS "LLVM_INCLUDE_DIRS=[${LLVM_INCLUDE_DIRS}]")

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

message(STATUS "LLVM_DEFINITIONS_LIST=[${LLVM_DEFINITIONS_LIST}]")

# LLVM library directory
execute_process(
    COMMAND llvm-config --libdir
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE LLVM_LIBRARY_DIR
)

message(STATUS "LLVM_LIBRARY_DIR=[${LLVM_LIBRARY_DIR}]")

# Find the libraries that correspond to the LLVM components
execute_process(
    COMMAND llvm-config --libs all
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE LLVM_LIBS
)

message(STATUS "LLVM_LIBS=[${LLVM_LIBS}]")

# SYSTEM: suppress warnings from LLVM headers (e.g. gcc14 false positive in SmallDenseMap)
target_include_directories(${SELF_LIB} SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(${SELF_LIB} PUBLIC ${LLVM_DEFINITIONS_LIST})
target_link_directories(${SELF_LIB} PUBLIC ${LLVM_LIBRARY_DIR})
target_link_libraries(${SELF_LIB} PUBLIC ${LLVM_LIBS})

# end CMakeLists.txt
