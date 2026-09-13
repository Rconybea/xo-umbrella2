{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-pyutil,
  xo-refcnt,
  xo-facet,
  xo-arena,
  xo-ppsink,
  # imported at module init: ArenaConfig and PpSink are registered there, and
  # pybind11 permits exactly one registration per c++ type
  xo-pyindentlog2,
  xo-pyarena,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyfacet";
    version = "1.0";

    src = ../xo-pyfacet;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-refcnt
      xo-facet
      xo-arena
      xo-ppsink
      xo-pyindentlog2
      xo-pyarena
    ];
  })
