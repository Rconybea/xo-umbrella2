{
  # dependencies
  stdenv, cmake, catch2, eigen,

  xo-cmake,
  xo-statistics,
  xo-reactor,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-kalmanfilter";
    version = "1.0";

    src = ../xo-kalmanfilter;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = false;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
      eigen
      xo-statistics
      xo-reactor
    ];
  })
