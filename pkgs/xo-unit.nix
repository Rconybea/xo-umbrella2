{
  # dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-ratio, xo-flatstring,

  xo-ppsink,
  # test-only xo dependencies
  xo-randomgen,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-unit";
    version = "1.0";

    src = ../xo-unit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit buildExamples;

    inherit doCheck;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    propagatedBuildInputs = [ xo-ratio xo-flatstring xo-ppsink ];

    nativeBuildInputs = [
      cmake
      catch2
      xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-randomgen
    ]
    ++ lib.optionals buildExamples [
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
