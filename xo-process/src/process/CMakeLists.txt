# xo-process/src/process/CMakeLists.txt

set(SELF_LIB process)
set(SELF_SRCS
    BrownianMotion.cpp ExpProcess.cpp Realization.cpp UpxEvent.cpp UpxToConsole.cpp
    init_process.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

# note: changes to xo_dependency() calls here
#       must coordinate with find_dependency() calls
#       in xo-process/cmake/processConfig.cmake.in
#
xo_dependency(${SELF_LIB} reactor)
xo_dependency(${SELF_LIB} printjson)
# BrownianMotion.cpp / ExpProcess.cpp include <xo/indentlog/scope.hpp>.
# Declared explicitly: xo-reactor and xo-printjson no longer propagate it.
xo_dependency(${SELF_LIB} indentlog)
#xo_dependency(${SELF_LIB} callback)
