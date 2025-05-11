{
  # nixpkgs dependencies
  stdenv, cmake, catch2, # ... other deps here

#  python3Packages,

  # xo dependencies
  xo-cmake,

  xo-simulator,
  #xo-refcnt,
  #xo-pyutil,
  #xo-reflect,

  xo-pyreactor,

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
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pysimulator";
    version = "1.0";

    outputs = [
      "out"
    ];

    src = ../xo-pysimulator;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;

    propagatedBuildInputs = [
#      xo-refcnt
      xo-pyreactor
      xo-simulator
#      xo-reflect
    ];

    nativeBuildInputs = [
      cmake
#      catch2
      xo-cmake
#      python3Packages.pybind11
    ];

  })
