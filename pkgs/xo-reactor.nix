{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-callback,
  xo-ppsink,
  xo-indentlog2,
  xo-reflect,
  xo-webutil,
  xo-printjson,
  xo-ordinaltree,

  # test-only xo dependencies
  xo-randomgen,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reactor";

    src = ../xo-reactor;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-randomgen
    ];
    propagatedBuildInputs = [
      xo-callback
      xo-ppsink
      xo-indentlog2
      xo-reflect
      xo-webutil
      xo-printjson
      xo-ordinaltree
    ];
  })
