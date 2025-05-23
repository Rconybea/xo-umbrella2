cmake_minimum_required(VERSION 3.10)

project(xo_macros VERSION 1.0)

# if any are useful for this project..
include (GNUInstallDirs)

set(XO_PROJECT_NAME xo_macros)

# LCOV_EXECUTABLE,GENHTML_EXECUTABLE: needed by xo-cmake-lcov-harness.in
find_program(LCOV_EXECUTABLE NAMES lcov)
find_program(GENHTML_EXECUTABLE NAMES genhtml)

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-cmake-lcov-harness.in
    ${PROJECT_BINARY_DIR}/xo-cmake-lcov-harness
    @ONLY
    )

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-cmake-config.in
    ${PROJECT_BINARY_DIR}/xo-cmake-config
    @ONLY
    )

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-build.in
    ${PROJECT_BINARY_DIR}/xo-build
    @ONLY
    )

install(
    FILES
    "cmake/xo_macros/xo-project-macros.cmake"
    "cmake/xo_macros/xo_cxx.cmake"
    "cmake/xo_macros/code-coverage.cmake"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/xo_macros
)

install(
    FILES
    "${PROJECT_BINARY_DIR}/xo-cmake-lcov-harness"
    "${PROJECT_BINARY_DIR}/xo-cmake-config"
    "${PROJECT_BINARY_DIR}/xo-build"
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES
    "etc/xo/subsystem-list"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${CMAKE_INSTALL_DATADIR}/etc/xo
)

# The cmake template gen-ccov.in should be expanded in downstream project;
# to pickup downstream project's PROJECT_SOURCE_DIR / PROJECT_BINARY_DIR
#
install(
    FILES
    "share/xo-macros/gen-ccov.in"
    "share/xo-macros/Doxyfile.in"
    "share/xo-macros/xo-bootstrap-macros.cmake"
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xo-macros
)
