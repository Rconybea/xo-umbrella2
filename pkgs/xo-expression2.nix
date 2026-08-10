{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, # cli11,

  # test-only: UtestRehearser, for the rendered-layout assertions in
  # utest/printable_render.test.cpp (same pattern as pkgs/xo-procedure2.nix)
  xo-testutil,

  # xo dependencies
  xo-numeric,
  xo-gc,
  xo-type,
  xo-reflect,
  xo-procedure2,
  xo-printable2,
  xo-flatstring,
  xo-subsys,
  xo-indentlog,
  xo-cmake,

  # config arguments
  doCheck ? true
} :

stdenv.mkDerivation (finalattrs :
  {
    name = "xo-expression2";

    src = ../xo-expression2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
      # cli11
    ];
    propagatedBuildInputs = [
      xo-numeric
      xo-gc
      xo-type
      xo-reflect
      xo-procedure2
      xo-printable2
      xo-flatstring
      xo-subsys
      xo-indentlog
    ];
  })
