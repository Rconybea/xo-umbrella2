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
xo_include_options(${SELF_LIBRARY_NAME})
xo_install_library(${SELF_LIBRARY_NAME})

# ----------------------------------------------------------------
# dependencies: logutil, ...

#xo_refcnt_dependency(${SELF_LIBRARY_NAME})
#xo_indentlog_dependency(${SELF_LIBRARY_NAME})

find_package(indentlog CONFIG REQUIRED)
#add_dependencies(${SELF_LIBRARY_NAME} indentlog)
# note:  can't use find_package() here,
#        because find_package() needs to run successfully before
#        dependency gets installed.
target_include_directories(
    ${SELF_LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/local/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC indentlog)

find_package(refcnt CONFIG REQUIRED)
#add_dependencies(${SELF_LIBRARY_NAME} refcnt)
target_include_directories(
    ${SELF_LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/local/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${SELF_LIBRARY_NAME} PUBLIC refcnt)

# ----------------------------------------------------------------
# 3rd party dependency: boost:

#xo_boost_dependency(${SELF_LIBRARY_NAME})

# end CMakeLists.txt
