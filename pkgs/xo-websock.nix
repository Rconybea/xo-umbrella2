{
  # nixpkgs dependencies
  stdenv, cmake, catch2, libwebsockets, jsoncpp,

  # xo dependencies
  xo-cmake,

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
      libwebsockets
      jsoncpp

      xo-reactor
#      xo-indentlog
    ];
  })
