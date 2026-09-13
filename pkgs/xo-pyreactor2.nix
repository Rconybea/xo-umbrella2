{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-reactor2,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyreactor2";
    version = "1.0";

    src = ../xo-pyreactor2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-reactor2
    ];
  })
