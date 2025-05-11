{
  # dependencies
  stdenv, cmake, catch2,

  xo-cmake,
  xo-refcnt,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-distribution";
    version = "1.0";

    src = ../xo-distribution;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-refcnt
    ];
  })
