{
  # dependencies
  lib, stdenv, cmake, catch2, replxx,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-expression, xo-tokenizer,
  xo-ppsink,
  # PrettySink, used by the exprreplxx example
  xo-indentlog2,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reader";
    version = "1.0";

    src = ../xo-reader;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    inherit doCheck;

    # PUBLIC deps, since exported xo_readerConfig.cmake does find_dependency() on them.
    propagatedBuildInputs = [ xo-expression
                              xo-tokenizer
                              xo-ppsink
                              xo-indentlog2
                            ];

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [ cmake
                           catch2
                           xo-cmake
                         ]
    ++ lib.optionals buildExamples [ replxx ]
    ++ lib.optionals buildDocs [
      doxygen
      sphinx
      graphviz
      python3Packages.sphinx-rtd-theme
      python3Packages.breathe
      python3Packages.sphinxcontrib-ditaa
      python3Packages.sphinxcontrib-plantuml
    ];
  })
