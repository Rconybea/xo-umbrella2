{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, replxx,

  # test-only: UtestRehearser, for the rendered-layout assertions in
  # utest/printable_render.test.cpp (same pattern as pkgs/xo-reader2.nix)
  xo-testutil,

  # xo dependencies
  xo-indentlog,
  xo-reader2,
  xo-gc,
  xo-cmake,
  xo-indentlog2,

  # config arguments
  doCheck ? true
} :

stdenv.mkDerivation (finalattrs :
  {
    name = "xo-interpreter2";

    src = ../xo-interpreter2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 replxx
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-indentlog
      xo-indentlog2
      xo-reader2
      xo-gc
    ];
  })
