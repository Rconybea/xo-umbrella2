{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies -- the four named by
  # xo-reactor2/src/reactor2/CMakeLists.txt:23-26
  xo-callback,
  xo-stringtable2,
  xo-reflect,
  xo-facet,
  # utest only
  xo-alloc2,
  xo-indentlog2,
  xo-printable2,
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
      xo-alloc2
      xo-indentlog2
      xo-printable2
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-callback
      xo-stringtable2
      xo-reflect
      xo-facet
    ];
  })
