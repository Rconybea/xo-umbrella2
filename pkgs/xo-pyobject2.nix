{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-pyutil,
  xo-refcnt,
  xo-object2,
  xo-facet,
  xo-printable2,
  xo-alloc2,
  xo-ppsink,
  # imported at module init
  xo-pyfacet,
  xo-pyindentlog2,
  # not named by pyobject2.cpp, but its python tests import xo.arena
  xo-pyarena,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyobject2";
    version = "1.0";

    src = ../xo-pyobject2;

    # ENABLE_TESTING alongside doCheck, not doCheck alone: utest/CMakeLists.txt
    # registers the python tests inside `if (ENABLE_TESTING)`, so without the
    # flag ctest runs and finds nothing -- reporting success having tested
    # nothing.  (22 other pkgs/*.nix still have exactly that defect.)
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    # The python tests import xo.arena, xo.indentlog2, xo.facet AND xo.object2,
    # but only xo.object2 is built here; the rest live in sibling store paths.
    #
    # This composes because `xo' is a PEP 420 namespace package
    # (.xo-backlog/python-packaging/issues/01): each of these directories holds
    # an xo/ with one module in it, and python merges them into one package
    # rather than letting the first shadow the others.  With an __init__.py
    # only one of these paths would be visible and the tests could not run here
    # at all.
    #
    # The generated xo-python wrapper PREPENDS this build tree's own python/
    # to whatever PYTHONPATH it inherits, so xo.object2 still resolves to the
    # module just built, not to an installed copy.
    preCheck = ''
      export PYTHONPATH=${xo-pyfacet}/lib/python:${xo-pyindentlog2}/lib/python:${xo-pyarena}/lib/python
    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-refcnt
      xo-object2
      xo-facet
      xo-printable2
      xo-alloc2
      xo-ppsink
      xo-pyfacet
      xo-pyindentlog2
      xo-pyarena
    ];
  })
