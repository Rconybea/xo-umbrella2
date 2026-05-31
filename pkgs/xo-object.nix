{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, libbsd,

  # xo dependencies
#  xo-reflect,
#  xo-webutil,
  xo-callback,
  xo-unit,
  xo-refcnt,
  xo-reflectutil,
  xo-alloc,
  xo-randomgen,
  xo-cmake,
#  xo-printjson,
#  xo-ordinaltree,
  xo-indentlog,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-object";

    src = ../xo-object;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 libbsd
      xo-randomgen
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-callback
      xo-unit
      xo-refcnt
#      xo-reflect
#      xo-webutil
#      xo-printjson
#      xo-ordinaltree
      xo-alloc
      xo-reflectutil
      xo-indentlog
    ];
  })
