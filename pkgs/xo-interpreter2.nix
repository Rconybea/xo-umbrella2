{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, replxx,

  # xo dependencies
  xo-reader2,
  xo-gc,
  xo-cmake,

  # config arguments
  doCheck ? true
} :

stdenv.mkDerivation (finalattrs :
  {
    name = "xo-interpreter2";

    src = ../xo-interpreter2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 replxx
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-reader2
      xo-gc
    ];
  })
