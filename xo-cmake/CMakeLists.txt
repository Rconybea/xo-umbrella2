cmake_minimum_required(VERSION 3.10)

project(xo_macros VERSION 1.0)

# if any are useful for this project..
include (GNUInstallDirs)

set(XO_PROJECT_NAME xo_macros)

# LCOV_EXECUTABLE,GENHTML_EXECUTABLE: needed by xo-cmake-lcov-harness.in (gcov toolchain)
find_program(LCOV_EXECUTABLE NAMES lcov)
find_program(GENHTML_EXECUTABLE NAMES genhtml)

# LLVM_PROFDATA_EXECUTABLE,LLVM_COV_EXECUTABLE: needed by xo-cmake-llvmcov-harness.in (llvm toolchain)
find_program(LLVM_PROFDATA_EXECUTABLE NAMES llvm-profdata)
find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov)

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-cmake-lcov-harness.in
    ${PROJECT_BINARY_DIR}/xo-cmake-lcov-harness
    @ONLY
    )
file(CHMOD ${PROJECT_BINARY_DIR}/xo-cmake-lcov-harness
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-cmake-llvmcov-harness.in
    ${PROJECT_BINARY_DIR}/xo-cmake-llvmcov-harness
    @ONLY
    )
file(CHMOD ${PROJECT_BINARY_DIR}/xo-cmake-llvmcov-harness
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

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

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-docker-build.in
    ${PROJECT_BINARY_DIR}/xo-docker-build
    @ONLY
    )

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-deps.in
    ${PROJECT_BINARY_DIR}/xo-deps
    @ONLY
    )
file(CHMOD ${PROJECT_BINARY_DIR}/xo-deps
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-gen-clang-format.in
    ${PROJECT_BINARY_DIR}/xo-gen-clang-format
    @ONLY
    )
file(CHMOD ${PROJECT_BINARY_DIR}/xo-gen-clang-format
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

configure_file(
    ${PROJECT_SOURCE_DIR}/bin/xo-clang-format-includes.in
    ${PROJECT_BINARY_DIR}/xo-clang-format-includes
    @ONLY
    )
file(CHMOD ${PROJECT_BINARY_DIR}/xo-clang-format-includes
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

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
    "${PROJECT_BINARY_DIR}/xo-cmake-llvmcov-harness"
    "${PROJECT_BINARY_DIR}/xo-cmake-config"
    "${PROJECT_BINARY_DIR}/xo-build"
    "${PROJECT_BINARY_DIR}/xo-docker-build"
    "${PROJECT_BINARY_DIR}/xo-deps"
    "${PROJECT_BINARY_DIR}/xo-gen-clang-format"
    "${PROJECT_BINARY_DIR}/xo-clang-format-includes"
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES
    "etc/xo/subsystem-list"
    "etc/xo/subsystem-edges"
    "etc/xo/dot-clang-format"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${CMAKE_INSTALL_DATADIR}/etc/xo
)

# ----------------------------------------------------------------
# Self-contained bootstrap snapshot of xo-cmake's own source.
# xo-docker-build (installed mode) bind-mounts this into the builder container
# and re-bootstraps an identical xo-cmake there, so the container's build
# tooling always matches this install.  The .in templates are installed
# unconfigured so they re-bake for the container prefix.
#
install(
    FILES CMakeLists.txt
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xo-cmake/src
)
install(
    DIRECTORY bin cmake etc share
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xo-cmake/src
    USE_SOURCE_PERMISSIONS
)

# The cmake template gen-ccov.in should be expanded in downstream project;
# to pickup downstream project's PROJECT_SOURCE_DIR / PROJECT_BINARY_DIR
#
install(
    FILES
    "share/xo-macros/gen-ccov.in"
    "share/xo-macros/Doxyfile.in"
    "share/xo-macros/xo-bootstrap-macros.cmake"
    "share/xo-macros/xo-reconfigure.in"
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xo-macros
)

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(utest)
endif()
