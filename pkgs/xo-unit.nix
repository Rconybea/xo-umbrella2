{
  # dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-ratio, xo-flatstring, xo-indentlog,

  buildDocs ? false,
  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-unit";
    version = "1.0";

    src = ../xo-unit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = true;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    propagatedBuildInputs = [ xo-ratio xo-flatstring ];

    nativeBuildInputs = [
      cmake
      catch2
      xo-cmake
    ]
    ++ lib.optionals buildExamples [
      xo-indentlog
    ]
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
