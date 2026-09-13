{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, cli11,

  # xo dependencies
  xo-testutil,
  xo-alloc2,
  xo-printable2,
  # AReflectable for DString, + the TaggedPtr it hands back
  xo-reflect,
  xo-reflectable2,
  # SetupStringtable2 registers DString's json printer; possible because
  # xo-printjson is levelled below xo-stringtable2
  xo-printjson,
  xo-arena,
  xo-cmake,
  xo-subsys,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-stringtable2";

    src = ../xo-stringtable2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-testutil
      cli11
    ];
    propagatedBuildInputs = [
      xo-subsys
      xo-alloc2
      xo-printable2
      xo-reflect
      xo-reflectable2
      xo-printjson
      xo-arena
    ];
  })
