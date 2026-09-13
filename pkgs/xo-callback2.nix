{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies -- the three named by
  # xo-callback2/CMakeLists.txt:38-40 (xo_headeronly_dependency)
  xo-alloc2,
  xo-callback,
  xo-facet,
  # utest only
  xo-ppsink,
  xo-testutil,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-callback2";

    src = ../xo-callback2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 cli11
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-ppsink
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-alloc2
      xo-callback
      xo-facet
    ];
  })
