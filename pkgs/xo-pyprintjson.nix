{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake,
  xo-printjson,

  xo-pyreflect,
  # ReflectAppcx is configure()'s second argument, and ReflectAppcx.hpp names
  # Indentlog2Appcx, so both types must be complete here
  xo-reflect, xo-indentlog2,
  # MemorySizeInfo, the element type visit_pools() returns
  xo-arena, xo-pyarena,
  # xo.indentlog2 -- the tests configure the stack from the bottom, and
  # xo.indentlog2 imports xo.arena at its own init
  xo-pyindentlog2,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyprintjson";

    src = ../xo-pyprintjson;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    # The tests import xo.indentlog2 and xo.reflect as well as xo.printjson,
    # and only xo.printjson is built here.  This composes because `xo' is a
    # PEP 420 namespace package (.xo-backlog/python-packaging/issues/01): each
    # store path holds an xo/ with one module in it, and python merges them
    # rather than letting the first shadow the rest.
    preCheck = ''
      export PYTHONPATH=${xo-pyreflect}/lib/python:${xo-pyindentlog2}/lib/python:${xo-pyarena}/lib/python
    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-printjson
      xo-pyreflect
      xo-reflect
      xo-indentlog2
      xo-arena
      xo-pyindentlog2
      xo-pyarena
    ];
  })
