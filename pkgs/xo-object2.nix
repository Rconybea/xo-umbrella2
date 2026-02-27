{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
#  xo-webutil,
#  xo-callback,
#  xo-unit,
#  xo-refcnt,
#  xo-reflectutil,
#  xo-alloc,
  #  xo-randomgen,
  xo-reflect,
  xo-gc,
  xo-printable2,
  xo-facet,
  xo-subsys,
  xo-cmake,
#  xo-printjson,
#  xo-ordinaltree,
#  xo-indentlog,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-object2";

    src = ../xo-object2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
#      xo-randomgen
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-reflect
      xo-gc
      xo-printable2
      xo-subsys
      xo-facet
    ];
  })
