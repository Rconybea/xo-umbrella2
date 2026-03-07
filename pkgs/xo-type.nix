{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
#  xo-reflectutil,
#  xo-alloc,
  #  xo-randomgen,
#  xo-gc,
#  xo-stringtable2,
  xo-alloc2,
#  xo-printable2,
  xo-facet,
  xo-subsys,
  xo-cmake,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-type";

    src = ../xo-type;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-alloc2
#      xo-printable2
      xo-subsys
      xo-facet
    ];
  })
