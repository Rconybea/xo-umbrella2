{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-refcnt,
  xo-randomgen,
#  xo-reflect,
    #, xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ordinaltree";

    src = ../xo-ordinaltree;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-refcnt
      xo-randomgen
    ];
  })
