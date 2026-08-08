{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake, xo-reflect, xo-flatstring,

  xo-ppsink,

  # test-only: PrettySink, for the rendered-layout assertions in
  # utest/pretty.test.cpp (same pattern as pkgs/xo-alloc.nix)
  xo-indentlog2,

  xo-refcnt,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-expression";
    version = "1.0";

    src = ../xo-expression;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit buildExamples;

    inherit doCheck;

    propagatedBuildInputs = [
                              xo-ppsink
                              xo-refcnt xo-reflect
                              xo-flatstring
                            ];
    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake
                        ] ++ lib.optionals doCheck [
                          xo-indentlog2
                        ];
  })
