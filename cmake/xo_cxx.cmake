
macro(xo_toplevel_compile_options)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 20)
    endif()
    if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
        set(CMAKE_CXX_STANDARD_REQUIRED True)
    endif()

    # ----------------------------------------------------------------
    # variable
    #   XO_ADDRESS_SANITIZE
    # determines whether to enable address sanitizer for the XO project
    # (see toplevel CMakeLists.txt)
    # ----------------------------------------------------------------
    if(XO_ADDRESS_SANITIZE)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()

    set(XO_STANDARD_COMPILE_OPTIONS -Werror -Wall -Wextra)
    # XO_ADDRESS_SANITIZE_COMPILE_OPTIONS: use when XO_ADDRESS_SANITIZE=ON
    #
    # address sanitizer build complains about _FORTIFY_SOURCE redefines
    #    In file included from <built-in>:460:
    #    <command line>:1:9: error: '_FORTIFY_SOURCE' macro redefined [-Werror,-Wmacro-redefined]
    #    #define _FORTIFY_SOURCE 2
    #
    set(XO_ADDRESS_SANITIZE_COMPILE_OPTIONS -Werror -Wall -Wextra -Wno-macro-redefined)

    if(XO_ADDRESS_SANITIZE)
        set(XO_COMPILE_OPTIONS ${XO_ADDRESS_SANITIZE_COMPILE_OPTIONS})
    else()
        set(XO_COMPILE_OPTIONS ${XO_STANDARD_COMPILE_OPTIONS})
    endif()

    # writes ${PROJECT_BINARY_DIR}/compile_commands.json;
    # (symlink from toplevel git dir to tell LSP how to build)
    #
    # note: trying to protect this with if(NOT DEFINED ..) is /not/ effective
    #
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

    if(NOT CMAKE_INSTALL_RPATH)
        set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING
            "runpath in installed libraries/executables")
    endif()
endmacro()

macro(xo_include_headeronly_options2 target)
    # ----------------------------------------------------------------
    # PROJECT_SOURCE_DIR:
    #   so we can for example write
    #     #include "ordinaltree/foo.hpp"
    #   from anywhere in the project
    # PROJECT_BINARY_DIR:
    #   since generated version file will be in build directory,
    #   need that build directory to also appear in
    #   compiler's include path
    #
    target_include_directories(
      ${target} INTERFACE
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src when ${target}=refcnt [DEPRECATED]
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/xo/${target}>    # e.g. for #include "TypeDescr.hpp" in reflect/src when ${target}=reflect
#      $<INSTALL_INTERFACE:include/xo/${target}>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>                      # e.g. for generated .hpp files
    )

    # ----------------------------------------------------------------
    # make standard directories for std:: includes explicit
    # so that
    # (1) they appear in compile_commands.json.
    # (2) clangd (run from emacs lsp-mode) can find them
    #
    if(CMAKE_EXPORT_COMPILE_COMMANDS)
      set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    endif()
endmacro()

# ----------------------------------------------------------------
# use this for a shared library.
#
macro(xo_add_shared_library target targetversion soversion sources)
    add_library(${target} SHARED ${sources})
    foreach(arg IN ITEMS ${ARGN})
        #message("target=${target}; arg=${arg}")

        # to use PUBLIC here would need to split:
        #   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${arg}}>
        #   $<INSTALL_INTERFACE:${arg}>
        # but shouldn't need that,  since we arrange to install includes via
        # xo_include_options2() below
        #
        target_sources(${target} PRIVATE ${arg})
    endforeach()
    set_target_properties(
        ${target}
        PROPERTIES
        VERSION ${targetversion}
        SOVERSION ${soversion})
    xo_compile_options(${target})
    xo_include_options2(${target})
    xo_install_library2(${target})
endmacro()

# ----------------------------------------------------------------
# use this in subdirs that compile c++ code.
# do not use for header-only subsystems;  see xo_include_headeronly_options2()
#
macro(xo_include_options2 target)
    # ----------------------------------------------------------------
    # PROJECT_SOURCE_DIR:
    #   so we can for example write
    #     #include "ordinaltree/foo.hpp"
    #   from anywhere in the project
    # PROJECT_BINARY_DIR:
    #   since generated version file will be in build directory,
    #   need that build directory to also appear in
    #   compiler's include path
    #
    target_include_directories(
      ${target} PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src [DEPRECATED]
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/xo/${target}>    # e.g. for #include "TypeDescr.hpp" in reflect/src when ${target}=reflect
#      $<INSTALL_INTERFACE:include/xo/${target}>
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>                      # e.g. for generated .hpp files
    )

    # ----------------------------------------------------------------
    # make standard directories for std:: includes explicit
    # so that
    # (1) they appear in compile_commands.json.
    # (2) clangd (run from emacs lsp-mode) can find them
    #
    if(CMAKE_EXPORT_COMPILE_COMMANDS)
      set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    endif()
endmacro()

# ----------------------------------------------------------------
#
# Require:
#     needs to be preceded by call to xo_toplevel_compile_options()
#
macro(xo_compile_options target)
    target_compile_options(${target} PRIVATE ${XO_COMPILE_OPTIONS})
endmacro()

# ----------------------------------------------------------------
# use this to install typical include file subtree
#
macro(xo_install_include_tree)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
endmacro()

# ----------------------------------------------------------------
# use this for a subdir that builds a library
# and supports find_package()
#
macro(xo_install_library2 target)
    install(
        TARGETS ${target}
        EXPORT ${target}Targets
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
    )
endmacro()

# ----------------------------------------------------------------

# for projectname=foo,  require:
#   cmake/fooConfig.cmake.in
#
# prepares
#   ${PREFIX}/lib/cmake/foo/fooConfig.cmake
#   ${PREFIX}/lib/cmake/foo/fooConfigVersion.cmake
#   ${PREFIX}/lib/cmake/foo/fooTargets.cmake
#
macro(xo_export_cmake_config projectname projectversion projecttargets)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${projectname}ConfigVersion.cmake"
        VERSION ${projectversion}
        COMPATIBILITY AnyNewerVersion
    )
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${projectname}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${projectname}Config.cmake"
        INSTALL_DESTINATION lib/cmake/${projectname}
    )
    install(
        EXPORT ${projecttargets}
        DESTINATION lib/cmake/${projectname}
    )
    install(
        FILES
        "${PROJECT_BINARY_DIR}/${projectname}ConfigVersion.cmake"
        "${PROJECT_BINARY_DIR}/${projectname}Config.cmake"
        DESTINATION lib/cmake/${projectname}
    )
endmacro()

# ----------------------------------------------------------------
#
# dependency on an xo library (including header-only libraries)
# e.g. indentlog
#
# An xo package foo works with cmake
#   find_package(foo)
# by providing plugin .cmake files in
#   ${PREFIX}/lib/cmake/foo/fooConfig.cmake
#   ${PREFIX}/lib/cmake/foo/fooConfigVersion.cmake
#   ${PREFIX}/lib/cmake/foo/fooTargets.cmake
#
# dep: name of required dependency,  e.g. indentlog
#
macro(xo_dependency target dep)
    find_package(${dep} CONFIG REQUIRED)
    target_link_libraries(${target} PUBLIC ${dep})
endmacro()

macro(xo_internal_dependency target dep)
  xo_dependency(${target} ${dep})
endmacro()

# dependency on a header-only library
#
macro(xo_dependency_headeronly target dep)
  find_package(${dep} CONFIG REQUIRED)
  target_link_libraries(${target} INTERFACE ${dep})
endmacro()

# dependency on namespaced target
# e.g.
#   add_library(foo ..) or add_executable(foo ...)
# then
#   xo_external_namespaced_dependency(foo Catch2 Catch2::Catch2)
# equivalent to
#   find_package(Catch2 CONFIG REQUIRED)
#   target_link_libraries(foo PUBLIC Catch2::Catch2)
#
macro(xo_external_target_dependency target pkg pkgtarget)
    find_package(${pkg} CONFIG REQUIRED)
    target_link_libraries(${target} PUBLIC ${pkgtarget})
endmacro()

# dependency on target provided from this codebase.
#
# 1. don't need find_package() in this case,  since details of dep targets
#    must be known to cmake for it to build them.
# 2. in any case, can't use find_package() when cmake runs,
#    because supporting .cmake files haven't been generated yet
#
macro(xo_self_dependency target dep)
    target_link_libraries(${target} PUBLIC ${dep})
endmacro()

# ----------------------------------------------------------------
#
