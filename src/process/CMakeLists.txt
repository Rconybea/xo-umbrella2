# xo-process/src/process/CMakeLists.txt

set(SELF_LIB process)
set(SELF_SRCS
    BrownianMotion.cpp ExpProcess.cpp Realization.cpp UpxEvent.cpp UpxToConsole.cpp
    init_process.cpp)

xo_add_shared_library3(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# note: changes to xo_dependency() calls here
#       must coordinate with find_dependency() calls
#       in xo-process/cmake/processConfig.cmake.in
#
xo_dependency(${SELF_LIB} reflect)
#xo_dependency(${SELF_LIB} webutil)
#xo_dependency(${SELF_LIB} callback)
