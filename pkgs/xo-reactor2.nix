{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-facet,
  xo-alloc2,
  xo-callback,
  xo-reflect,
  xo-indentlog2,
  xo-printable2,
  xo-stringtable2,
  xo-testutil,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reactor2";

    src = ../xo-reactor2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 cli11
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-facet
      xo-alloc2
      xo-callback
      xo-reflect
      xo-indentlog2
      xo-printable2
      xo-stringtable2
    ];
  })
