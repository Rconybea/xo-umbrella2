{
  # dependencies
  stdenv, cmake, catch2,
  clang, llvm,
  doxygen,

  python3Packages,
  sphinx,

  xo-cmake, xo-tokenizer, xo-expression,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-jit";
    version = "1.0";

    stdenv = stdenv;

    src = ../xo-jit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [
      cmake
      catch2
      clang
      llvm.dev
      doxygen
      sphinx
      xo-cmake
      xo-tokenizer
    ];
    propagatedBuildInputs = [
      xo-expression
    ];
  })
