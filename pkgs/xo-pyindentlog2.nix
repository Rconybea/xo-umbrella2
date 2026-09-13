{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-pyutil,
  xo-refcnt,
  xo-indentlog2,
  xo-arena,
  # imported at module init, for the single permitted ArenaConfig registration
  xo-pyarena,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyindentlog2";
    version = "1.0";

    src = ../xo-pyindentlog2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-refcnt
      xo-indentlog2
      xo-arena
      xo-pyarena
    ];
  })
