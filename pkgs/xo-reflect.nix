{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake, xo-refcnt, xo-subsys,
  xo-ppsink,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reflect";

    src = ../xo-reflect;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [ xo-subsys xo-refcnt xo-ppsink ];
  })
