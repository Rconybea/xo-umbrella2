# xo-flatstring/docs/CMakeLists.txt

if (XO_SUBMODULE_BUILD)
    # in submodule build, rely on toplevel docs/CMakeLists.txt file instead
else()
    # build docs starting from here only in standalone build.
    # otherwise use top-level doxygen setup instead.

    set(ALL_LIBRARY_TARGETS xo_flatstring)  # todo: automate this from xo-cmake macros
    set(ALL_UTEST_TARGETS xo_flatstring_ex1 ) # todo: automate this from xo-cmake macros

    # look for doxygen executable
    find_program(DOXYGEN_EXECUTABLE NAMES doxygen REQUIRED)
    message("-- DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE}")

    # look for sphinx-build executable
    find_program(SPHINX_EXECUTABLE NAMES sphinx-build REQUIRED)
    message("-- SPHINX_EXECUTABLE=${SPHINX_EXECUTABLE}")

    set(DOX_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    set(DOX_INPUT_DIR ${PROJECT_SOURCE_DIR})
    set(DOX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/dox)

    set(DOX_INDEX_FILE ${DOX_OUTPUT_DIR}/html/index.html)

    # .hpp files reachable from xo-flatstring/include
    #
    # REMINDER: for reliability will need to re-run cmake when the set of .hpp files changes
    #
    file(GLOB_RECURSE DOX_HPP_FILES_GLOB ${PROJECT_SOURCE_DIR}/include *.hpp)

    set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/sphinx/html)
    set(SPHINX_INDEX_FILE ${SPHINX_OUTPUT_DIR}/index.html)
    #
    # sphinx .rst files reachable from cmake-examples/docs
    #
    # REMINDER: for reliability will need to re-run cmake when the set of .rst files changes
    #
    file(GLOB_RECURSE SPHINX_RST_FILES_GLOB ${CMAKE_CURRENT_SOURCE_DIR} *.rst)

    set(SPHINX_RST_FILES index.rst install.rst lessons.rst flatstring-reference.rst flatstring-class.rst)

    # TODO:
    # 1. move Doxyfile.in to xo-cmake project
    # 2. replace this command section with xo-cmake macro
    #
    configure_file(
        Doxyfile.in ${DOX_CONFIG_FILE}
        FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        @ONLY)

    set(DOX_DEPS ${ALL_LIBRARY_TARGETS} ${ALL_UTEST_TARGETS} ${DOX_HPP_FILES_GLOB})

    file(MAKE_DIRECTORY ${DOX_OUTPUT_DIR})
    add_custom_command(
        OUTPUT ${DOX_INDEX_FILE}
        DEPENDS ${DOX_DEPS}
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
        DEPENDS ${DOX_INDEX_FILE} ${DOX_DEPS}
    )

    # root of sphinx doc tree
    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
    set(SPHINX_DEPS doxygen conf.py ${SPHINX_RST_FILES} ${SPHINX_RST_FILES_GLOB} ${DOX_DEPS})

    add_custom_command(
        OUTPUT ${SPHINX_INDEX_FILE}
        DEPENDS ${SPHINX_DEPS}
        COMMAND ${SPHINX_EXECUTABLE}
        -b html -Dbreathe_projects.xodoxxml=${CMAKE_CURRENT_BINARY_DIR}/dox/xml
        ${SPHINX_SOURCE} ${SPHINX_OUTPUT_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating docs (sphinx) -> [${SPHINX_OUTPUT_DIR}]")

    # make sphinx --> generate sphinx documentation
    #
    add_custom_target(
        sphinx
        DEPENDS ${SPHINX_INDEX_FILE})

    # - html docs generated in build/docs/sphinx
    # - copy the doc tree to share/doc/xo_unit/html
    #
    #  OPTIONAL:    install directory tree if it exists,
    #               but don't complain if it's missing
    install(
        DIRECTORY ${SPHINX_OUTPUT_DIR}
        FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}
        COMPONENT Documentation
        OPTIONAL)

    # make docs --> generate sphinx documentation
    add_custom_target(
        docs
        DEPENDS sphinx)
endif()
