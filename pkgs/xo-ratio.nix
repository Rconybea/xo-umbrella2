{
  # dependencies
  lib, stdenv, cmake, catch2, cli11,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-flatstring, xo-reflectutil,

  xo-ppsink,
  # test-only xo dependencies
  xo-reflect, xo-randomgen, xo-indentlog2,
  xo-testutil,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ratio";
    version = "1.0";

    src = ../xo-ratio;

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

    # xo-ratio is header-only; its exported cmake config + public headers pull in
    # these, so consumers (e.g. xo-unit) must receive them transitively.
    propagatedBuildInputs = [
      xo-ppsink
      xo-flatstring
      xo-reflectutil
    ];

    nativeBuildInputs = [
      cmake
      xo-cmake
    ]
    ++ lib.optionals doCheck [
      xo-reflect
      xo-randomgen
      xo-indentlog2
      xo-testutil
      catch2
      cli11
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
      #python3Packages.sphinxcontrib-plantuml
    ];
  })
