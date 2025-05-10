{
  # dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake, xo-indentlog
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-refcnt";
    version = "1.0";

    src = ../xo-refcnt;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    propagatedBuildInputs = [ xo-indentlog ];
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
  })
