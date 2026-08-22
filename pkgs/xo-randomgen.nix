{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-ppsink,

  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-randomgen";

    src = ../xo-randomgen;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildExamples;

    doCheck = true;

    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    # headers include <xo/ppsink/..>, so consumers need it too
    propagatedBuildInputs = [ xo-ppsink ];
  })
