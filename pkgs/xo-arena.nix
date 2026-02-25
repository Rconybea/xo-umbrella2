{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-randomgen,
  xo-reflectutil,
  xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-arena";

    src = ../xo-arena;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DENABLE_TESTING=1"
                 ];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2
      xo-cmake xo-randomgen
    ];
    propagatedBuildInputs = [
      xo-reflectutil
      xo-indentlog
    ];
  })
