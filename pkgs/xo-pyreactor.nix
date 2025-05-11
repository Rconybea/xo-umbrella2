{
  # nixpkgs dependencies
  stdenv, cmake, catch2, # ... other deps here

  python3Packages,

  # xo dependencies
  xo-cmake,

  xo-reactor,
  #xo-webutil,
  #xo-refcnt,
  #xo-pyutil,
  #xo-reflect,

  xo-pyprintjson,

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
    name = "xo-pyreactor";
    version = "1.0";

    outputs = [
      "out"
    ];

    src = ../xo-pyreactor;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;

    propagatedBuildInputs = [
#      xo-refcnt
#      xo-pyutil
      xo-pyprintjson
      xo-reactor
#      xo-reflect
    ];

    nativeBuildInputs = [
      cmake
#      catch2
      xo-cmake
#      python3Packages.pybind11
    ];

  })
