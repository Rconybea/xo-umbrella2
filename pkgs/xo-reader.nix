{
  # dependencies
  stdenv, cmake, catch2, replxx,

  xo-cmake, xo-expression, xo-tokenizer
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reader";
    version = "1.0";

    src = ../xo-reader;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake catch2
                          xo-cmake
                          xo-expression
                          xo-tokenizer
                        ];
  })
