{
  # dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake,

  buildDocs ? false,
  buildExamples ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-indentlog";
    version = "1.0";

    src = ../xo-indentlog;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                 ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=on"];

    inherit buildDocs;
    inherit buildExamples;

    doCheck = true;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake ]
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
