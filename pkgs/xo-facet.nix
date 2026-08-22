{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-cmake,
  xo-reflectutil,
  xo-arena,
  xo-indentlog2,
  xo-ppsink,
  # utest/ only: UtestAppStart owns the single catch2 runtime, and the suite
  # installs a ThreadPrettySink
  xo-testutil,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-facet";

    src = ../xo-facet;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake
      catch2
      cli11
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-reflectutil
      xo-arena
      xo-indentlog2
      xo-ppsink
    ];
  })
