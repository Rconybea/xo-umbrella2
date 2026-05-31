{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-allocutil,
  xo-refcnt,
  xo-randomgen,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ordinaltree";

    src = ../xo-ordinaltree;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-allocutil
      xo-refcnt
      xo-randomgen
    ];
  })
