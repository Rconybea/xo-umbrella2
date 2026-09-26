{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-cmake,
  xo-reactor,
  xo-websock,
  xo-webutil,
  # test-only: shared utest main (UtestAppStart)
  xo-testutil,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reactor2websock";

    src = ../xo-reactor2websock;

    # ENABLE_TESTING alongside doCheck: utest/CMakeLists.txt registers the
    # tests inside `if (ENABLE_TESTING)`, so without it ctest finds nothing
    # and reports success.
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-testutil
    ];
    propagatedBuildInputs = [
      xo-reactor
      xo-websock
      xo-webutil
    ];
  })
