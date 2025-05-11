{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake, xo-refcnt,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyutil";

    src = ../xo-pyutil;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    #doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
      xo-refcnt
      python3Packages.python
      python3Packages.pybind11
    ];
  })
