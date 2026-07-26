{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake, xo-timeutil,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ppsink";
    version = "1.0";

    src = ../xo-ppsink;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    # xo-ppsink's exported config + public headers pull in xo-timeutil,
    # so consumers must receive it transitively.
    propagatedBuildInputs = [ xo-timeutil ];

    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake ];
  })
