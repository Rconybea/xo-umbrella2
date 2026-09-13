{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, # ... other deps here

  python3Packages,

  # xo dependencies
  xo-cmake, xo-refcnt, xo-pyutil, xo-reflect,
  # Indentlog2Appcx is configure()'s second argument; xo-pyindentlog2 registers
  # it with pybind11 and this module imports that at init
  xo-indentlog2, xo-pyindentlog2,
  # MemorySizeInfo, the element type visit_pools() returns
  xo-arena, xo-pyarena,

  # args

  #   attrset for fetching source code.
  #    { type, owner, repo, ref }
  #
  #   e.g. type="github", owner="rconybea", repo="cmake-examples", ref="ex1b"
  #
  #   see [[../flake.nix]]
  #
  #cmake-examples-ex1-path

  # someconfigurationoption ? false

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyreflect";
    version = "1.0";

    outputs = [
      "out"
    ];

    src = ../xo-pyreflect;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    preCheck = ''
      export PYTHONPATH=${xo-pyindentlog2}/lib/python:${xo-pyarena}/lib/python
    '';

    propagatedBuildInputs = [
      xo-pyutil
      xo-reflect
      xo-indentlog2
      xo-arena
      xo-pyindentlog2
      xo-pyarena
    ];

    nativeBuildInputs = [
      cmake
      xo-cmake
      python3Packages.pybind11
    ];

  })
