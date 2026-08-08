{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake, xo-reflect, xo-flatstring,

  xo-indentlog,

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
                              xo-indentlog
                              xo-refcnt xo-reflect
                              xo-flatstring
                            ];
    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake
                        ];
  })
