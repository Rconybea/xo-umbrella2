{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  xo-procedure2,
  xo-subsys,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-numeric";

    src = ../xo-numeric;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-procedure2
      xo-subsys
    ];
  })
