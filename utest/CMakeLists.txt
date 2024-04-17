# xo-flatstring/utest/CMakeLists.txt

set(SELF_EXE utest.flatstring)
set(SELF_SRCS
    flatstring_utest_main.cpp
    flatstring.test.cpp)

add_executable(${SELF_EXE} ${SELF_SRCS})
xo_include_options2(${SELF_EXE})

add_test(NAME ${SELF_EXE} COMMAND ${SELF_EXE})
#target_code_coverage(${SELF_EXE} AUTO ALL)

# ----------------------------------------------------------------
# in coverage build,  target to build+install coverage report

set(CCOV_OUTPUT_DIR ${PROJECT_BINARY_DIR}/ccov/html)
set(CCOV_INDEX_FILE ${CCOV_OUTPUT_DIR}/index.html)
set(CCOV_REPORT_EXE ${PROJECT_BINARY_DIR}/gen-ccov)
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

# ----------------------------------------------------------------
# deps:  logutils, ...

xo_self_headeronly_dependency(${SELF_EXE} xo_flatstring)
xo_dependency(${SELF_EXE} indentlog)
xo_external_target_dependency(${SELF_EXE} Catch2 Catch2::Catch2)

# end CMakeLists.txt
