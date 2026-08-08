{
  # dependencies
  lib, stdenv, cmake, catch2,

  clang, llvm,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-tokenizer, xo-expression,

  xo-reflectutil,

  # test-only xo dependencies
  xo-ratio,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-jit";
    version = "1.0";

    stdenv = stdenv;

    src = ../xo-jit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = false; # tests disabled, until segfault resolved
    #inherit doCheck;

    propagatedBuildInputs = [
      xo-reflectutil
      xo-expression
    ];

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [
      cmake
      catch2
      clang
      llvm.dev
      doxygen
      sphinx
      xo-cmake
      xo-tokenizer
    ] ++ lib.optionals doCheck [
      xo-ratio
    ] ++ lib.optionals buildDocs [
      doxygen
      sphinx
      graphviz
      python3Packages.sphinx-rtd-theme
      python3Packages.breathe
      python3Packages.sphinxcontrib-ditaa
      python3Packages.sphinxcontrib-plantuml
    ];

  })
