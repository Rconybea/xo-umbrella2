{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  xo-type,
  xo-object2,
  xo-subsys,
  xo-indentlog,
  xo-cmake,

  # test-only: UtestRehearser, for the rendered-layout assertions in
  # utest/printable_render.test.cpp (same pattern as pkgs/xo-object2.nix)
  xo-testutil,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-procedure2";

    src = ../xo-procedure2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
      cli11
    ];
    propagatedBuildInputs = [
      xo-indentlog
      xo-type
      xo-object2
      xo-subsys
    ];
  })
