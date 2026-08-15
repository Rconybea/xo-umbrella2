{
  # dependencies
  lib, stdenv, cmake, catch2, eigen,

  xo-cmake,

  xo-indentlog2,
  xo-statistics,
  xo-reactor,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-kalmanfilter";
    version = "1.0";

    src = ../xo-kalmanfilter;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
      xo-indentlog2
      eigen
      xo-statistics
      xo-reactor
    ];
  })
