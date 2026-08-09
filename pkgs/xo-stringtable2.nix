{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-testutil,
  xo-alloc2,
  xo-printable2,
  xo-arena,
  xo-cmake,
  xo-indentlog,
  xo-subsys,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-stringtable2";

    src = ../xo-stringtable2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DENABLE_TESTING=1"
                 ];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-testutil
      cli11
    ];
    propagatedBuildInputs = [
      xo-indentlog
      xo-subsys
      xo-alloc2
      xo-printable2
      xo-arena
    ];
  })
