# xo-flatstring/utest/CMakeLists.txt

set(SELF_EXE utest.flatstring)
set(SELF_SRCS
    flatstring_utest_main.cpp
    flatstring.test.cpp)

xo_add_utest_executable(${SELF_EXE} ${SELF_SRCS})

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
# deps:  logutils, ...

xo_self_headeronly_dependency(${SELF_EXE} xo_flatstring)
xo_dependency(${SELF_EXE} indentlog)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
