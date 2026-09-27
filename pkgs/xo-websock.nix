{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, libwebsockets, jsoncpp,

  # xo dependencies
  xo-cmake,
  xo-webutil,

  xo-ppsink,
  xo-printjson,

  # test-only: xo-websock/utest/CMakeLists.txt
  xo-testutil,
  # tests and examples (xo-websock/example/introspect)
  xo-indentlog2,

  doCheck ? true,
  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-websock";

    src = ../xo-websock;

    # without -DENABLE_TESTING=1 the utest targets are not built, and ctest
    # passes on "No tests were found!!!" -- see pkgs/xo-alloc2.nix
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];
    inherit doCheck;
    inherit buildExamples;

    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
    ] ++ lib.optionals (doCheck || buildExamples) [
      xo-indentlog2
    ];
    propagatedBuildInputs = [
      xo-webutil
      xo-ppsink
      libwebsockets
      jsoncpp

      xo-printjson
    ];
  })
