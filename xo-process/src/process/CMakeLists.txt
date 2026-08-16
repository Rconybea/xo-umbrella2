# xo-process/src/process/CMakeLists.txt

set(SELF_LIB process)
set(SELF_SRCS
    BrownianMotion.cpp ExpProcess.cpp Realization.cpp UpxEvent.cpp UpxToConsole.cpp
    init_process.cpp)

xo_add_shared_library4(${SELF_LIB} ${PROJECT_NAME}Targets ${PROJECT_VERSION} 1 ${SELF_SRCS})

# ----------------------------------------------------------------
# external dependencies

xo_dependency(${SELF_LIB} reactor)
xo_dependency(${SELF_LIB} printjson)
# RealizationSource.hpp / RealizationCallback.hpp name xo::pp:: in public
# headers, so this is PUBLIC.
xo_dependency(${SELF_LIB} xo_ppsink)
# UpxEvent.cpp uses xo::pp::tostr; no public header needs indentlog2.
xo_dependency(${SELF_LIB} xo_indentlog2)
#xo_dependency(${SELF_LIB} callback)
