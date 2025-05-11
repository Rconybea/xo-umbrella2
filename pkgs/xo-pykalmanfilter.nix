{
  # nixpkgs dependencies
  stdenv, cmake, catch2, # ... other deps here

  # xo dependencies
  xo-cmake,
  xo-kalmanfilter,

  xo-pyutil,
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
    name = "xo-pykalmanfilter";
    version = "1.0";

    src = ../xo-pykalmanfilter;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-pyreactor
      xo-kalmanfilter
    ];
  })
