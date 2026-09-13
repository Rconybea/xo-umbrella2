{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-cmake,
  xo-ppsink,
#  xo-refcnt,
  xo-reflect,
  xo-reflectable2,
  # test-only: UtestAppStart/UtestListener, see utest/printjson_utest_main.cpp
  xo-testutil,
  # test-only: that same main builds an Indentlog2Appcx directly
  xo-indentlog2,
  # test-only: a D-type that is printable but NOT reflectable
  xo-printable2,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-printjson";

    src = ../xo-printjson;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
      xo-indentlog2
      xo-printable2
      cli11
    ];
    propagatedBuildInputs = [
      xo-ppsink
      xo-reflect
      xo-reflectable2
    ];
  })
