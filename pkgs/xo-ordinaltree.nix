{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-allocutil,
  xo-refcnt,
  xo-randomgen,
  xo-cmake,

  xo-indentlog2,
  xo-ppsink,
  # test-only xo dependencies
  xo-object,
  xo-alloc,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ordinaltree";

    src = ../xo-ordinaltree;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-object
      xo-alloc
    ];
    propagatedBuildInputs = [
      xo-indentlog2
      xo-ppsink
      xo-allocutil
      xo-refcnt
      xo-randomgen
    ];
  })
