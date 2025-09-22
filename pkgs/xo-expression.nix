{
  # dependencies
  lib, stdenv, cmake, catch2,

  xo-cmake, xo-reflect, xo-flatstring,

  buildDocs ? false,
  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-expression";
    version = "1.0";

    src = ../xo-expression;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = true;

    propagatedBuildInputs = [ xo-reflect
                              xo-flatstring
                            ];
    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake
                        ];
  })
