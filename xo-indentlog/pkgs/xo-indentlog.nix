{
  # dependencies
  stdenv, cmake, catch2,

  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-indentlog";
    version = "1.0";

    src = ../xo-indentlog;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
  })
