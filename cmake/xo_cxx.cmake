
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
    # NOTE: using INTERFACE here is mandatory.  Otherwise get error:
    #        target_include_directories may only set INTERFACE properties on INTERFACE targets
    #
    target_include_directories(
      ${target} INTERFACE
      $<INSTALL_INTERFACE:include>
      $<INSTALL_INTERFACE:include/xo/${target}>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src when ${target}=refcnt [DEPRECATED]
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/xo/${target}>    # e.g. for #include "TypeDescr.hpp" in reflect/src when ${target}=reflect
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
# use this to introduce a shared library.
# - has symlink-enabled .hpp install
#
macro(xo_add_shared_library4 target projectTargets targetversion soversion sources)
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
    xo_install_library4(${target} ${projectTargets})
endmacro()

# ----------------------------------------------------------------
# OBSOLETE.  prefer xo_add_shared_library4()
#
macro(xo_add_shared_library3 target projectTargets targetversion soversion sources)
    message(WARNING "obsolete call to xo_add_shared_library3(); prefer xo_add_shared_library4()")

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
    xo_install_library3(${target} ${projectTargets})
endmacro()

# ----------------------------------------------------------------
# OBSOLETE.  prefer xo_add_shared_library3()
#
macro(xo_add_shared_library target targetversion soversion sources)
    message(WARNING "obsolete call to xo_add_shared_library(); prefer xo_add_shared_library4()")

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
# use this for a header-only library
#
macro(xo_add_headeronly_library target)
    add_library(${target} INTERFACE)
    xo_include_headeronly_options2(${target})
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
      $<INSTALL_INTERFACE:include>
      $<INSTALL_INTERFACE:include/xo/${target}>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>              # e.g. for #include "indentlog/scope.hpp"
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${target}>    # e.g. for #include "Refcounted.hpp" in refcnt/src [DEPRECATED]
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/xo/${target}>    # e.g. for #include "TypeDescr.hpp" in reflect/src when ${target}=reflect
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
#
# establish default value for XO_SYMLINK_INSTALL.
#
# may want to use this for a nested build,
# where we want to run cmake for nested codebase using externalproject_add().
#
# in this case:
# 1. will need to build+install nested project foo to temporary location in build tree,
#    so that build artifacts (such as fooConfig.cmake) are available to depending
#    projects
# 2. bona fide install with (for example) copied .hpp files interferes with
#    cross-codebase development.
#    2a. want changes to original .hpp files to trigger rebuild of depending codebases.
#        This won't happen if depending project relies on a copy
#    2b. want IDE that observes compiler commands (i.e. LSP) to visit .hpp files
#        in their original codebase,  since that's the correct place to make any edits.
#
# see
# - xo_install_include_tree()
#
macro(xo_establish_symlink_install)
    if(NOT DEFINED XO_SYMLINK_INSTALL)
        set(XO_SYMLINK_INSTALL False)
    endif()
endmacro()

# ----------------------------------------------------------------
# use this to install typical include file subtree
#
macro(xo_install_include_tree)
    message(WARNING "deprecated xo_install_include_tree(); prefer xo_install_include_tree3()")

    xo_establish_symlink_install()

    if(XO_SYMLINK_INSTALL)
        message(FATAL_ERROR "symlink install not implemented for ${PROJECT_SOURCE_DIR}/include -- use xo_install_include_tree3()")
    else()
        install(
            DIRECTORY ${PROJECT_SOURCE_DIR}/include/
            FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
    endif()
endmacro()

# create symlink from ${symlink_path} -> ${dest_path},
# from
#   make install
#
macro(xo_install_make_symlink dest_path symlink_dir symlink_name)
    install(CODE "message(\"make_directory: ${symlink_dir}\")")
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${symlink_dir})")

    install(CODE "message(\"symlink: ${symlink_dir}/${symlink_name} -> ${dest_path}/${symlink_name}\")")
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${dest_path}/${symlink_name} ${symlink_dir}/${symlink_name})")
endmacro()

# e.g. path = include/xo/foo to install xo-foo/include/xo/foo
#
macro(xo_install_include_tree3 subdir_path)
    xo_establish_symlink_install()

    if(XO_SYMLINK_INSTALL)
        # ugh.  cmake doesn't allow input path argument to cmake_path()
        # to be a macro variable.
        set(_xo_install_include_tree3_subdir_path ${subdir_path})
        set(_xo_install_include_tree3_dirname "")
        set(_xo_install_include_tree3_basename "")
        cmake_path(GET _xo_install_include_tree3_subdir_path PARENT_PATH _xo_install_include_tree3_dirname)
        cmake_path(GET _xo_install_include_tree3_subdir_path FILENAME _xo_install_include_tree3_basename)

        xo_install_make_symlink(
            ${PROJECT_SOURCE_DIR}/${_xo_install_include_tree3_dirname}
            ${CMAKE_INSTALL_PREFIX}/${_xo_install_include_tree3_dirname}
            ${_xo_install_include_tree3_basename})
    else()
        install(
            DIRECTORY ${PROJECT_SOURCE_DIR}/${subdir_path}
            FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${subdir_path})
    endif()
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

macro(xo_install_library3 target projectTargets)
    install(
        TARGETS ${target}
        EXPORT ${projectTargets}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
    )

    xo_install_include_tree()
endmacro()

macro(xo_install_library4 target projectTargets)
    install(
        TARGETS ${target}
        EXPORT ${projectTargets}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
    )

    xo_install_include_tree3(include/xo/${target})

    #xo_install_include_tree() -- use xo_install_include_tree3() separately
endmacro()

macro(xo_install_library5 target nxo_target projectTargets)
    install(
        TARGETS ${target}
        EXPORT ${projectTargets}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        RUNTIME DESTINATION bin COMPONENT Runtime
        PUBLIC_HEADER DESTINATION include COMPONENT Development
        BUNDLE DESTINATION bin COMPONENT Runtime
    )

    xo_install_include_tree3(include/xo/${nxo_target})

    #xo_install_include_tree() -- use xo_install_include_tree3() separately
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
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${projectname})
    install(
        FILES
        "${PROJECT_BINARY_DIR}/${projectname}ConfigVersion.cmake"
        "${PROJECT_BINARY_DIR}/${projectname}Config.cmake"
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${projectname}
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

# dependency of a header-only library on another header-only library
#
macro(xo_headeronly_dependency target dep)
  find_package(${dep} CONFIG REQUIRED)
  # Conflict here between PUBLIC and INTERFACE
  #
  # PUBLIC ensures that include directories required by ${dep} will also be included in compilation of ${target};
  # i.e. will appear in property ${target}.INCLUDE_DIRECTORIES
  #
  # INTERFACE mandatory when depending on a header-only library (created with add_library(foo INTERFACE)).
  # otherwise get error:
  #   INTERFACE library can only be used with the INTERFACE keyword of
  #   target_link_libraries
  # Unfortunately target_link_libraries() does not copy dependent's INTERFACE_INCLUDE_DIRECTORIES property
  # (at least asof cmake 3.25.3).  Dependent's INCLUDE_DIRECTORIES property will be empty,  since it's header-only.
  #
  # Workaround by copying property explicity,  which we do below
  #
  target_link_libraries(${target} INTERFACE ${dep})

#  get_target_property(xo_dependency_headeronly__tmp ${dep} INTERFACE_INCLUDE_DIRECTORIES)
#  set_property(
#      TARGET ${target}
#      APPEND PROPERTY INCLUDE_DIRECTORIES ${xo_dependency_headeronly__tmp})
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
# need this when linking pybind11-generated libraries
#
macro(xo_pybind11_link_flags)
    # see:
    # 1. FAQ Build Issues Q2
    # 2. CMAKE_SHARED_LINKER_FLAGS in src/CMakeLists.txt
    # 3. pybind11 cmake support, somewhere like
    #   [path/to/pybind11-2.9.2/
    #     lib/python3.9-pybind11-2.9.2/
    #     lib/python3.9/site-packages/
    #     pybind11/share/cmake/
    #     pybind11/pybind11Common.cmake]
    #
    set_property(
      TARGET pybind11::python_link_helper
      APPEND
      PROPERTY
      INTERFACE_LINK_OPTIONS "$<$<PLATFORM_ID:Darwin>:LINKER:-flat_namespace>")

    # looks like pybind11_add_module() tries to link transitive deps
    # of libs mentioned in xo_dependency() -- perhaps for link-time optimization?
    #
    # For example when linking libpyreflect, get link line with -lrefcnt
    # (presumably since cmake knows that libreflect.so depends on librefcnt.so);
    # this triggers error,  since link doesn't know where to find librefcnt.so
    #
    # To workaround,  add ${CMAKE_INSTALL_PREFIX}/lib to the link line.
    #
    # WARNING: expect this not to work in a hermetic nix build!
    #
    set_property(TARGET pybind11::python_link_helper
        APPEND
        PROPERTY
        INTERFACE_LINK_OPTIONS "-L${CMAKE_INSTALL_PREFIX}/lib")
endmacro()

# ----------------------------------------------------------------
# use this for a subdir that builds a python library using pybind11
#
# expecting the following
# 1. a directory pyfoo/ -> library pyfoo
# 2. pyfoo/pyfoo.hpp.in -> pyfoo/pyfoo.hpp
#
macro(xo_pybind11_library target projectTargets source_files)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include/xo/${target})

    configure_file(
        ${target}.hpp.in
        ${PROJECT_BINARY_DIR}/include/xo/${target}/${target}.hpp)
    # was ${PROJECT_SOURCE_DIR}/include/xo/${target}/${target}.hpp)

    xo_establish_symlink_install()

    if(XO_SYMLINK_INSTALL)
        xo_install_make_symlink(
            ${PROJECT_BINARY_DIR}/include/xo/${target}
            ${CMAKE_INSTALL_PREFIX}/include/xo/${target}
            ${target}.hpp)
    else()
        install(
            FILES ${PROJECT_BINARY_DIR}/include/xo/${target}/${target}.hpp
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include/xo/${target})
    endif()

    # find_package(Python..) finds python in
    #   /Library/Frameworks/Python.framework/...
    # but we want to use python from nix
    #
    #find_package(Python COMPONENTS Interpreter Development REQUIRED)
    #

    find_package(pybind11)

    # this only works if one source file, right?
    #
    # 6oct2023
    # Having trouble at link time with this.
    # Getting broken link for nix,  because link line lists short library nicknames
    # (e.g. -lfoo) for transitive deps.  nix link needs to be given the directory
    # in which libfoo.so resides,  so we need to ensure full path
    #
    # - source files:
    #     -fPIC -fvisibility=hidden -flto -fno-fat-lto-objects
    # - library:
    #     -fPIC -flto
    #     (transitive closure of library deps for lto?)
    #
    pybind11_add_module(${target} MODULE ${source_files})

    xo_pybind11_link_flags()
    xo_include_options2(${target})
    xo_install_library4(${target} ${projectTargets})
endmacro()

# ----------------------------------------------------------------
# use this for a dependency of a pybind11 library,
# e.g. that was introduced by xo_pybind11_library()
#
# Working around the following problem (cmake 3.25.3, pybind11 2.10.4).N
# if:
# 1. we have pybind11 library pyfoo,  depending on c++ native library foo.
# 2. foo depends on other libraries foodep1, foodep2;
#    assume also that foodep2 is header-only
#
# if we write:
#   # CMakeLists.txt
#   pybind11_add_module(pyfoo MODULE pyfoo.cpp)
#   find_package(foo CONFIG_REQUIRED)
#   target_link_libraries(pyfoo PUBLIC foo)
#
# get compile instructions like:
#   g++ -o pyfoo.cpython-311-x86_64-linux-gnu.so path/to/pyfoo.cpp.o path/to/libfoo.so.x.y -lfoodep1 -lfoodep2
#
# 1. This is broken for foodep2,  since there no libfoodep2.so exists
# 2. Also broken for nix build,  because directory containing libfoodep1.so doesn't appear on the compile line.
#    (It's likely possible to extract this from the .cmake package in lib/cmake/foo/fooTargets.cmake,
#     but I don't know how to do that yet)
#
# workaround here is to suppress these secondary dependencies.
# This assumes:
# 1. secondary dependencies are all in shared libraries (not needed on link line)
# 2. (maybe?) primary dependency libfoo.so is sufficient to satisfy g++
#    -- conceivably true if libfoo.so has RUNPATH etc.
#
macro(xo_pybind11_dependency target dep)
    find_package(${dep} CONFIG REQUIRED)
    # clobber secondary dependencies, as discussed above
    set_property(TARGET ${dep} PROPERTY INTERFACE_LINK_LIBRARIES "")
    # now that secondary deps are gone,  attach to target pybind11 library
    # skip xo_dependency() here,  that would repeat the find_package() expansion
    target_link_libraries(${target} PUBLIC ${dep})
endmacro()
