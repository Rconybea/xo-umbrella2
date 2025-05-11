{
  # dependencies
  stdenv, cmake, catch2,

  xo-cmake, xo-indentlog
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-tokenizer";
    version = "1.0";

    src = ../xo-tokenizer;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    propagatedBuildInputs = [ ];
    nativeBuildInputs = [ cmake catch2
                          xo-cmake
                          xo-indentlog
                        ];
  })
