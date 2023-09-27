# reflect/CMakeLists.txt

set(SELF_LIBRARY_NAME reflect)
set(SELF_SOURCE_FILES TypeDescr.cpp TypeDescrExtra.cpp TaggedRcptr.cpp atomic/AtomicTdx.cpp pointer/PointerTdx.cpp vector/VectorTdx.cpp struct/StructTdx.cpp struct/StructMember.cpp init_reflect.cpp)

# build shared library 'reflect'
add_library(${SELF_LIBRARY_NAME} SHARED ${SELF_SOURCE_FILES})

set_target_properties(${SELF_LIBRARY_NAME}
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)

# ----------------------------------------------------------------
# all the errors+warnings!
#
#target_compile_options(${SELF_LIBRARY_NAME} PRIVATE -Werror -Wall -Wextra)
xo_compile_options(${SELF_LIBRARY_NAME})
xo_include_options2(${SELF_LIBRARY_NAME})
xo_install_library2(${SELF_LIBRARY_NAME})

# ----------------------------------------------------------------
# dependencies: logutil, ...

xo_internal_dependency(${SELF_LIBRARY_NAME} indentlog)
xo_internal_dependency(${SELF_LIBRARY_NAME} refcnt)

# ----------------------------------------------------------------
# 3rd party dependency: boost:

#xo_boost_dependency(${SELF_LIBRARY_NAME})

# end CMakeLists.txt
