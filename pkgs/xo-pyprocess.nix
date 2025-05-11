{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake,
  #xo-refcnt,
  xo-process,

  xo-pyreactor,
  xo-pywebutil,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyprocess";

    src = ../xo-pyprocess;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    #doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
#      xo-printjson
      xo-pyreactor
      xo-pywebutil
      xo-process
#      python3Packages.python
      python3Packages.pybind11
    ];
  })
