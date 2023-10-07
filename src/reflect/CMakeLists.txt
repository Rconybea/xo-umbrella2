# reflect/CMakeLists.txt

set(SELF_LIBRARY_NAME reflect)
set(SELF_SOURCE_FILES TypeDescr.cpp TypeDescrExtra.cpp TaggedRcptr.cpp atomic/AtomicTdx.cpp pointer/PointerTdx.cpp vector/VectorTdx.cpp struct/StructTdx.cpp struct/StructMember.cpp init_reflect.cpp)

xo_add_shared_library(${SELF_LIBRARY_NAME} ${PROJECT_VERSION} 1 ${SELF_SOURCE_FILES})

# ----------------------------------------------------------------
# dependencies: indentlog, ...

# note: changes here must coordinate with cmake/reflectConfig.cmake.in
xo_dependency_headeronly(${SELF_LIBRARY_NAME} indentlog)
xo_dependency(${SELF_LIBRARY_NAME} refcnt)

# ----------------------------------------------------------------
# 3rd party dependency: boost:

#xo_boost_dependency(${SELF_LIBRARY_NAME})

# end CMakeLists.txt
