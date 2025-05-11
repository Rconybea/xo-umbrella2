{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  #python3Packages,

  # xo dependencies
  xo-cmake,
  #xo-refcnt,
  xo-distribution,

  xo-pyutil,
  #xo-pyreflect,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pydistribution";

    src = ../xo-pydistribution;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    #doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
      xo-distribution
      xo-pyutil
#      xo-pyreflect
#      xo-refcnt
#      python3Packages.python
#      python3Packages.pybind11
    ];
  })
