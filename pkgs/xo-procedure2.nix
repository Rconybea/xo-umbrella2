{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  xo-object2,
  xo-subsys,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-procedure2";

    src = ../xo-procedure2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-object2
      xo-subsys
    ];
  })
