{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-ppsink,
  xo-indentlog2,
  xo-printjson,
  xo-reactor,
  xo-simulator,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-process";

    src = ../xo-process;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-ppsink
      xo-indentlog2
      xo-printjson
      xo-reactor
      xo-simulator
    ];
  })
