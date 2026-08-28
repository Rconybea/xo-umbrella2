# xo/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(xo-umbrella VERSION 1.0)

# Adopting submodule builds directly into this cmake.
# Want this before including bootstrap macros,
# so local ./xo-cmake supersedes installed version
#
# Submodule builds will pickup dependent xo artifacts directly
# from sibling build dirs.
# (Contrast with a build that relies on install step).
# In particular,  configure step in satellite projects
# needs to avoid using cmake find_package() on sibling xo projects:
# 1. .cmake support files
#      fooConfig.cmake
#      fooConfigVersion.cmake
#      fooTargets.cmake
#    won't have been installed
# 2. In any case,  they point to final install location;
#    we need build location
#
set(XO_SUBMODULE_BUILD True)

# ----------------------------------------------------------------
# global build settings

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/xo-cmake/cmake)

include(cmake/xo-bootstrap-macros.cmake)

# toplevel source directory;  used only with XO_SUBMODULE_BUILD
set(XO_UMBRELLA_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(XO_UMBRELLA_REPO_SUBDIR .)
# toplevel binary directory;  used only with XO_SUBMODULE_BUILD
set(XO_UMBRELLA_BINARY_DIR ${CMAKE_BINARY_DIR})

xo_cxx_toplevel_options3()

# ----------------------------------------------------------------

# temporary compiler flags here
set(PROJECT_CXX_FLAGS "")
add_definitions(${PROJECT_CXX_FLAGS})

# NB c++ standard comes from xo_toplevel_compile_options() (xo-cmake), which
# xo_cxx_toplevel_options3() above has already run -- currently 23.
# Override with -DCMAKE_CXX_STANDARD=N.

set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(NOT CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "runpath for installed libraries/executables")
endif()

message("-- CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
message("-- CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
message("-- CMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}")
message("-- XO_ENABLE_DOCS=${XO_ENABLE_DOCS}")
message("-- XO_ENABLE_EXAMPLES=${XO_ENABLE_EXAMPLES}")
message("-- XO_ENABLE_VULKAN=${XO_ENABLE_VULKAN}")
message("-- XO_ENABLE_OPENGL=${XO_ENABLE_OPENGL}")
message("-- XO_ENABLE_ASM=${XO_ENABLE_ASM}")

# ----------------------------------------------------------------
#
set(DOX_EXCLUDE_PATTERNS [=[
*/utest/* \
*/llvm/* \
*/llvm-c/*
]=])

# ----------------------------------------------------------------
# xo satellite projects
# in reverse topological order i.e. dependencies first

add_subdirectory(xo-cmake)
add_subdirectory(xo-timeutil)     # time vocabulary + calendar conveniences
add_subdirectory(xo-ppsink)       # arena-free pretty-print front-end (PpSink/FlatSink/Pretty)
add_subdirectory(xo-indentlog)
add_subdirectory(xo-subsys)
add_subdirectory(xo-flatstring)
add_subdirectory(xo-reflectutil)  # header-only reflect support
add_subdirectory(xo-testutil)     # unit test aux functions (UtestRehearser).
add_subdirectory(xo-randomgen)    # xoshiro256ss.
                                  # NB after xo-testutil: utest.randomgen uses it
add_subdirectory(xo-arena)        # arena allocator (DArena)
add_subdirectory(xo-indentlog2)   # arena-backed version of indentlog
add_subdirectory(xo-facet)        # sep iface,data
add_subdirectory(xo-printable2)   # experiment w/ facet object model (fomo)
add_subdirectory(xo-alloc2)       # arena api (fomo)
add_subdirectory(xo-refcnt)
add_subdirectory(xo-pyutil)
add_subdirectory(xo-reflect)      # reflection library (pre-fomo)
add_subdirectory(xo-pyreflect)
add_subdirectory(xo-stringtable2) # experiment w/ facet object model
add_subdirectory(xo-object2)      # experiment w/ facet object model
add_subdirectory(xo-gc)           # incremental gc + facet object model
add_subdirectory(xo-type)         # experiment w/ facet object model
add_subdirectory(xo-procedure2)   # schematika procedure abstraction + runtime context (fomo)
add_subdirectory(xo-equable2)     # equality facet -- scaffolded, no methods yet
add_subdirectory(xo-hashable2)    # hashing facet -- scaffolded, no methods yet
add_subdirectory(xo-numeric)      # schematika poly arithmetic w/ facet object model
add_subdirectory(xo-tokenizer2)   # schematika tokenizer (w/ fomo)
add_subdirectory(xo-expression2)  # schematika expressions (fomo)
#
add_subdirectory(xo-allocutil)    # xo-alloc api
add_subdirectory(xo-ratio)
add_subdirectory(xo-unit)         # constexpr dimension checking
add_subdirectory(xo-callback)
add_subdirectory(xo-callback2)    # callback set over explicit obj<AAllocator> (fomo)
add_subdirectory(xo-reactor2)     # reactor (w/ fomo)
add_subdirectory(xo-alloc)        # arena allocator + incremental gc
add_subdirectory(xo-pyunit)
#add_subdirectory(xo-reflect2)     # experiment w/ facet object model
add_subdirectory(xo-object)
add_subdirectory(xo-ordinaltree)
#
add_subdirectory(xo-reader2)      # schematika expression parser (fomo)
add_subdirectory(xo-interpreter2) # schematika interpreter (fomo)
#
add_subdirectory(xo-webutil)
add_subdirectory(xo-pywebutil)
add_subdirectory(xo-printjson)
add_subdirectory(xo-pyprintjson)
add_subdirectory(xo-reactor)
add_subdirectory(xo-pyreactor)
add_subdirectory(xo-websock)
add_subdirectory(xo-pywebsock)
#
add_subdirectory(xo-statistics)
add_subdirectory(xo-distribution)
add_subdirectory(xo-pydistribution)
add_subdirectory(xo-simulator)
add_subdirectory(xo-pysimulator)
add_subdirectory(xo-process)
add_subdirectory(xo-pyprocess)
add_subdirectory(xo-kalmanfilter)
add_subdirectory(xo-pykalmanfilter)
#
add_subdirectory(xo-expression)
add_subdirectory(xo-pyexpression)
add_subdirectory(xo-tokenizer)
add_subdirectory(xo-reader)
#add_subdirectory(xo-symboltable)
add_subdirectory(xo-interpreter)
add_subdirectory(xo-jit)
add_subdirectory(xo-pyjit)   
#
add_subdirectory(xo-imgui)

# ----------------------------------------------------------------
# umbrella-wide facet code generation

xo_umbrella_genfacet_all(xo-genfacet-all)

# ----------------------------------------------------------------
# umbrella-wide coverage target + utest binary manifest.
# no-op unless CMAKE_BUILD_TYPE=coverage.

xo_umbrella_coverage_config()

# ----------------------------------------------------------------
# documentation.  must follow add_subdirectory() for satellite projects

xo_umbrella_doxygen_deps(
    xo_arena
    xo_facet
    xo_alloc2
    indentlog
    xo_flatstring
    xo_ratio
    xo_unit
    xo_tokenizer
    xo_reader
    xo_interpreter
    xo_jit)
xo_umbrella_doxygen_config()
xo_umbrella_sphinx_config(
    index.rst
    docs/install.rst
    docs/glossary.rst)

# ----------------------------------------------------------------
# inter-subsystem dependency graph, for maintaining the canonical bottom-up
# order shared by xo-cmake/etc/xo/subsystem-list and ci.nix.
#   tsort ${CMAKE_BINARY_DIR}/subsystem-edges   # -> dependencies-first order
#
# NB the basename depends on provisioning: a fully-enabled configure writes
# subsystem-edges, a partial one writes subsystem-partial-edges.  See
# xo_emit_dependency_edges().

xo_emit_dependency_edges(${CMAKE_BINARY_DIR} XO_SUBSYSTEM_EDGES_FILE)

# candidate .clang-format, whose xo include-ordering categories are derived
# (descending topological order) from the same edge list.  Adopt with:
#   cp ${CMAKE_BINARY_DIR}/dot-clang-format ${CMAKE_SOURCE_DIR}/.clang-format
execute_process(
    COMMAND ${CMAKE_BINARY_DIR}/xo-cmake/xo-gen-clang-format
            ${XO_SUBSYSTEM_EDGES_FILE}
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/dot-clang-format
    RESULT_VARIABLE _xo_dot_clang_format_rc)
if(_xo_dot_clang_format_rc EQUAL 0)
    message(STATUS "xo-gen-clang-format: wrote ${CMAKE_BINARY_DIR}/dot-clang-format"
                   " (adopt: cp -> ${CMAKE_SOURCE_DIR}/.clang-format)")
else()
    message(WARNING "xo-gen-clang-format: failed (rc=${_xo_dot_clang_format_rc})")
endif()
