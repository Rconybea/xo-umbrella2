{
  # dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-reflectutil,
  xo-indentlog,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-refcnt";
    version = "1.0";

    src = ../xo-refcnt;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    inherit doCheck;

    propagatedBuildInputs = [
      xo-reflectutil
      xo-indentlog
    ];
    nativeBuildInputs = [
      cmake
      catch2
      xo-cmake ];
  })
