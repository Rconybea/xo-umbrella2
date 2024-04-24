# xo-unit/utest/CMakeLists.txt

set(SELF_EXECUTABLE_NAME utest.unit)
set(SELF_SOURCE_FILES
    unit_utest_main.cpp  #mpl_unit.test.cpp
    unit.test.cpp #quantity.test.cpp
)

add_executable(${SELF_EXECUTABLE_NAME} ${SELF_SOURCE_FILES})
xo_include_options2(${SELF_EXECUTABLE_NAME})
add_test(NAME ${SELF_EXECUTABLE_NAME} COMMAND ${SELF_EXECUTABLE_NAME})

# ----------------------------------------------------------------
# in coverage build,  target to build+install coverage report

if (XO_SUBMODULE_BUILD)
    # in submodule build, generate aggregate coverage report
    # for all xo libraries
else()
    set(CCOV_OUTPUT_DIR ${PROJECT_BINARY_DIR}/ccov/html)
    set(CCOV_INDEX_FILE ${CCOV_OUTPUT_DIR}/index.html)
    set(CCOV_REPORT_EXE ${PROJECT_BINARY_DIR}/gen-ccov)
    # CMAKE_INSTALL_DOCDIR
    #  =default=> DATAROOTDIR/doc/PROJECT_NAME
    #  =default=> CMAKE_INSTALL_PREFIX/share/doc/xo_flatstring
    set(CCOV_INSTALL_DOCDIR ${CMAKE_INSTALL_DOCDIR}/ccov)

    # 'test' target should always be out-of-date
    #
    # DEPENDS: reminder - can't put 'test' here,  requires 'all' target
    #
    add_custom_command(
        OUTPUT ${CCOV_INDEX_FILE}
        DEPENDS ${SELF_EXE}
        COMMAND ${CCOV_REPORT_EXE}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating coverage report -> [${CCOV_OUTPUT_DIR}]")

    add_custom_target(
        ccov
        DEPENDS ${CCOV_INDEX_FILE} ${SELF_EXE})

    # OPTIONAL: quietly skip this step if ccov report not generated
    install(
        DIRECTORY ${CCOV_OUTPUT_DIR}
        FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        DESTINATION ${CCOV_INSTALL_DOCDIR}
        COMPONENT Documentation
        OPTIONAL)
endif()

# ----------------------------------------------------------------
# dependencies..

xo_self_dependency(${SELF_EXECUTABLE_NAME} xo_unit)
xo_external_target_dependency(${SELF_EXECUTABLE_NAME} Catch2 Catch2::Catch2)

# end CMakeLists.txt
