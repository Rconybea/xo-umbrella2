{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-ppsink,
#  xo-refcnt,
  xo-reflect,
    #, xo-indentlog,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-printjson";

    src = ../xo-printjson;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
xo-ppsink
#      xo-refcnt
      xo-reflect
#      xo-indentlog
    ];
  })
