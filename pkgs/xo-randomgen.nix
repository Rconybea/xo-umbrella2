{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake, xo-indentlog,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-randomgen";

    src = ../xo-randomgen;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [ xo-indentlog ];
  })
