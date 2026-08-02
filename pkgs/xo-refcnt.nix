{
  # dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-reflectutil,
  xo-ppsink,
  xo-indentlog,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-refcnt";
    version = "1.0";

    src = ../xo-refcnt;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    propagatedBuildInputs = [
      xo-reflectutil
      xo-ppsink
      xo-indentlog
    ];
    nativeBuildInputs = [
      cmake
      catch2
      xo-cmake ];
  })
