{
  # nixpkgs dependencies
  stdenv, cmake, catch2, libwebsockets, jsoncpp,

  # xo dependencies
  xo-cmake,
  xo-webutil,

  xo-ppsink,
  xo-reactor,

} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-websock";

    src = ../xo-websock;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-webutil
      xo-ppsink
      libwebsockets
      jsoncpp

      xo-reactor
    ];
  })
