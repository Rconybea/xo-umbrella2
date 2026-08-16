{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-reflectutil,
  xo-arena,
  xo-indentlog2,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-facet";

    src = ../xo-facet;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DENABLE_TESTING=1"
                 ];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-reflectutil
      xo-arena
      xo-indentlog2
    ];
  })
