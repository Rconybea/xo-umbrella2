{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-facet,
  xo-reflect,
  xo-testutil,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reflectable2";

    src = ../xo-reflectable2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 cli11
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-facet
      xo-reflect
    ];
  })
