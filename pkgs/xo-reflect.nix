{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-testutil,
  xo-refcnt,
  xo-subsys,
  xo-ppsink,
  xo-cmake,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reflect";

    src = ../xo-reflect;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [ cmake catch2 xo-cmake ]
                        ++ lib.optionals doCheck [
                          xo-testutil
                          cli11
                        ]
    ;
    propagatedBuildInputs = [ xo-subsys xo-refcnt xo-ppsink ];
  })
