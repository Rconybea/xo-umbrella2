{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, replxx,

  xo-stringtable2,
  xo-arena,
  xo-indentlog,
  xo-cmake,

  buildExamples ? true,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-tokenizer2";

    src = ../xo-tokenizer2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildExamples;
    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 replxx
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-stringtable2
      xo-arena
      xo-indentlog
    ];
  })
