{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
#  xo-refcnt,
#  xo-reflect,
  #  xo-reactor,
  xo-simulator,
    #, xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-process";

    src = ../xo-process;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-simulator
#      xo-refcnt
#      xo-reflect
#      xo-indentlog
    ];
  })
