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
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reader";
    version = "1.0";

    src = ../xo-reader;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = true;

    # PUBLIC deps: xo_readerConfig.cmake does find_dependency() on these, so a
    # consumer must be able to resolve them -- nativeBuildInputs would satisfy
    # xo-reader's own build but propagate nothing.  Latent until xo-interpreter
    # was packaged (2026-08-08) and became xo-reader's first nix consumer.
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
