{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-reactor,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-simulator";

    src = ../xo-simulator;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-reactor
    ];
  })
