{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
#  xo-refcnt,
  xo-reflect,
    #, xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-printjson";

    src = ../xo-printjson;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
#      xo-refcnt
      xo-reflect
#      xo-indentlog
    ];
  })
