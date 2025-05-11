{
  # dependencies
  stdenv, cmake, catch2,

  xo-cmake, xo-reflect,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-expression";
    version = "1.0";

    src = ../xo-expression;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    propagatedBuildInputs = [ xo-reflect ];
    nativeBuildInputs = [ cmake catch2
                          xo-cmake
                        ];
  })
