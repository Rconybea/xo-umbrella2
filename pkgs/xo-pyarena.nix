{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-pyutil,
  xo-refcnt,
  xo-arena,
  xo-ppsink,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyarena";
    version = "1.0";

    src = ../xo-pyarena;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-refcnt
      xo-arena
      xo-ppsink
    ];
  })
