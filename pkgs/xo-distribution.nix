{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake,

  xo-ppsink,
  xo-refcnt,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-distribution";
    version = "1.0";

    src = ../xo-distribution;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-ppsink
      xo-refcnt
    ];
  })
