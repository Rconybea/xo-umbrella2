# ----------------------------------------------------------------
# for example use
#   $ PREFIX=/usr/local   # for example
#   $ cmake -DCMAKE_MODULE_PATH=prefix -DCMAKE_INSTALL_PREFIX=$PREFIX -B .build
#
# will set
#   CMAKE_MODULE_PATH = /usr/local/share/cmake
# and expect .cmake macros in
#   /usr/local/share/cmake/xo_macros/xo-project-macros.cmake
# ----------------------------------------------------------------

if (("${CMAKE_MODULE_PATH}" STREQUAL "") OR ("${CMAKE_MODULE_PATH}" STREQUAL prefix))
    # default to typical install location for xo-project-macros
    set(CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/share/cmake)
endif()

if (NOT XO_SUBMODULE_BUILD)
    message("-- GUESSED_CMAKE_CMD=cmake -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH} -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR} -B ${CMAKE_BINARY_DIR}")
    message("-- CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
    message("-- CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
endif()

# needs to have been installed somewhere on CMAKE_MODULE_PATH,
# (e.g. from xo-cmake with the same value for CMAKE_INSTALL_PREFIX)
#
#include(xo_macros/xo-project-macros)
include(xo_macros/xo_cxx)   # not using v1 code-coverage; testing cmake-examples impl instead
