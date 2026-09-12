{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-cmake,
  xo-ppsink,
#  xo-refcnt,
  xo-reflect,
  # test-only: utest/FopJson.test.cpp reflects a faceted object pointer.  The
  # library edge printjson -> xo-reflectable2 arrives with the entry point in
  # .xo-backlog/reflectable2/issues/04.
  xo-reflectable2,
  # test-only: UtestAppStart/UtestListener, see utest/printjson_utest_main.cpp
  xo-testutil,
  # test-only: that same main builds an Indentlog2Appcx directly
  xo-indentlog2,

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
      xo-reflectable2
      xo-testutil
      xo-indentlog2
      cli11
    ];
    propagatedBuildInputs = [
      xo-ppsink
      xo-reflect
    ];
  })
