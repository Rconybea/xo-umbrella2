{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake,
  #xo-refcnt,
  xo-websock,

  xo-pyreflect,
  xo-pywebutil
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pywebsock";

    src = ../xo-pywebsock;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    #doCheck = true;
    nativeBuildInputs = [ cmake catch2 xo-cmake ];
    propagatedBuildInputs = [
#      xo-printjson
      xo-pywebutil
      xo-pyreflect
      xo-websock
#      python3Packages.python
#      python3Packages.pybind11
    ];
  })
