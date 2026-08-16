{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-ppsink,
  xo-indentlog2,
  xo-callback,
  xo-refcnt,
#  xo-reflect,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-webutil";

    src = ../xo-webutil;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-ppsink
      xo-indentlog2
      xo-callback
      xo-refcnt
#      xo-reflect
    ];
  })
