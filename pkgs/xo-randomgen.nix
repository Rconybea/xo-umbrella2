{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake, xo-indentlog,

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
    propagatedBuildInputs = [ xo-indentlog ];
  })
