{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-timeutil";
    version = "1.0";

    src = ../xo-timeutil;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake ];
  })
