{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-reflect,
  xo-webutil,
  xo-printjson,
  xo-ordinaltree,
    #, xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reactor";

    src = ../xo-reactor;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-reflect
      xo-webutil
      xo-printjson
      xo-ordinaltree
#      xo-indentlog
    ];
  })
