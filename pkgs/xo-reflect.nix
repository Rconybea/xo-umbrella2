{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake, xo-refcnt, xo-subsys
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reflect";

    src = ../xo-reflect;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [ xo-subsys xo-refcnt ];
  })
