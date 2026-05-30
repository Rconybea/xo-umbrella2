{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-allocutil,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-allocutil";

    src = ../xo-allocutil;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DENABLE_TESTING=1"
                 ];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
    ];
  })
