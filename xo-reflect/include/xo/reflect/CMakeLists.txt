# reflect/CMakeLists.txt

set(SELF_LIBRARY_NAME reflect)

# build shared library 'reflect'
add_library(${SELF_LIBRARY_NAME} SHARED TypeDescr.cpp TypeDescrExtra.cpp TaggedRcptr.cpp atomic/AtomicTdx.cpp pointer/PointerTdx.cpp vector/VectorTdx.cpp struct/StructTdx.cpp struct/StructMember.cpp init_reflect.cpp)

set_target_properties(${SELF_LIBRARY_NAME} PROPERTIES
              VERSION ${PROJECT_VERSION}
              SOVERSION 1
              PUBLIC_HEADER TypeDescr.hpp)

# ----------------------------------------------------------------
# all the errors+warnings!
#
#target_compile_options(${SELF_LIBRARY_NAME} PRIVATE -Werror -Wall -Wextra)
xo_compile_options(${SELF_LIBRARY_NAME})
xo_include_options(${SELF_LIBRARY_NAME})

# ----------------------------------------------------------------
# internal dependencies: logutil, ...

target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC refcnt)

# ----------------------------------------------------------------
# 3rd party dependency: boost:

#xo_boost_dependency(${SELF_LIBRARY_NAME})

xo_install_library(${SELF_LIBRARY_NAME})

# end CMakeLists.txt
