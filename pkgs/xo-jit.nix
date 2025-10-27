{
  # dependencies
  lib, stdenv, cmake, catch2,

  clang, llvm,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-tokenizer, xo-expression,

  buildDocs ? false,
  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-jit";
    version = "1.0";

    stdenv = stdenv;

    src = ../xo-jit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = true;

    propagatedBuildInputs = [
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
