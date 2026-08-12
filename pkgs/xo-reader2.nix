{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # test-only: UtestRehearser, for the rendered-layout assertions in
  # utest/printable_render.test.cpp (same pattern as pkgs/xo-expression2.nix)
  xo-testutil,

  # xo dependencies
  xo-numeric,
  xo-procedure2,
  xo-type,
  xo-expression2,
  xo-tokenizer2,
  xo-gc,
  xo-subsys,
  xo-cmake,

  # config arguments
  doCheck ? true
} :

stdenv.mkDerivation (finalattrs :
  {
    name = "xo-reader2";

    src = ../xo-reader2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-numeric
      xo-procedure2
      xo-type
      xo-expression2
      xo-tokenizer2
      xo-gc
      xo-subsys
    ];
  })
