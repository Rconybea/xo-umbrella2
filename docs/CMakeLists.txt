# xo-unit/docs/CMakeLists.txt

# copied from xo-observable/docs/CMakeLists.txt

set(DOX_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#set(DOX_DEPS ${PROJECT_SOURCE_DIR}/mainpage.dox)   # not yet
set(DOX_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/dox)

set(DOX_INDEX_FILE ${DOX_OUTPUT_DIR}/html/index.html)

#set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/sphinx/html)
#set(SPHINX_INDEX_FILE ${SPHINX_OUTPUT_DIR}/index.html)
#
## sphinx .rst files reachable from cmake-examples/docs
#file(GLOB_RECURSE SPHINX_RST_FILES ${CMAKE_CURRENT_SOURCE_DIR} *.rst)

set(ALL_LIBRARY_TARGETS xo_unit)  # todo: automate this from xo-cmake macros
#set(ALL_UTEST_TARGETS "")        # todo: automate this from xo-cmake macros

# look for doxygen executable
find_program(DOXYGEN_EXECUTABLE NAMES doxygen REQUIRED)
message("-- DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE}")

if (XO_SUBMODULE_BUILD)
    # in submodule build, rely on toplevel docs/CMakeLists.txt file instead
else()
    # build docs starting from here only in standalone build.
    # otherwise use top-level doxygen setup instead.

    # TODO:
    # 1. move Doxyfile.in to xo-cmake project
    # 2. replace this command section with xo-cmake macro
    #
    configure_file(
        Doxyfile.in ${DOX_CONFIG_FILE}
        FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        @ONLY)

    file(MAKE_DIRECTORY ${DOX_OUTPUT_DIR})
    add_custom_command(
        OUTPUT ${DOX_INDEX_FILE}
        DEPENDS ${DOX_DEPS} ${ALL_LIBRARY_TARGETS} ${ALL_UTEST_TARGETS}
        COMMAND "${DOXYGEN_EXECUTABLE}" ${DOX_CONFIG_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY ${DOX_CONFIG_FILE}
        COMMENT "Generating docs (doxygen)")

    # To build this target
    #   $ cmake --build .build -j -- doxygen
    # or
    #   $ cd .build
    #   $ make doxygen
    #
    add_custom_target(
        doxygen
        DEPENDS ${DOX_INDEX_FILE}
    )
endif()
