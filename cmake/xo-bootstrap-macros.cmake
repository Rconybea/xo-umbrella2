if (("${CMAKE_MODULE_PATH}" STREQUAL "") OR ("${CMAKE_MODULE_PATH}" STREQUAL "prefix"))
    # default to typical install location for xo-project-macros
    set(CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/share/cmake)
endif()

if (NOT XO_SUBMODULE_BUILD)
    message("-- GUESSED_CMAKE_CMD=cmake -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH} -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -B ${CMAKE_BINARY_DIR}")
    message("-- CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
    message("-- CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
endif()

# needs to have been installed somewhere on CMAKE_MODULE_PATH,
# (e.g. from xo-cmake with the same value for CMAKE_INSTALL_PREFIX)
#
include(xo_macros/xo-project-macros)
