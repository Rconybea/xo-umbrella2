{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,

} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-subsys";
    version = "1.0";

    src = ../xo-subsys;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    #doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
  })
