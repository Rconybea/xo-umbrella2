{
  # dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  doxygen, sphinx, graphviz, fontconfig,

  xo-cmake,
  xo-timeutil,

  buildDocs ? false,
  buildExamples ? false,
  doCheck ? true,
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
    inherit doCheck;

    preBuild = lib.optionalString buildDocs ''
      #echo "FONTCONFIG_FILE=$FONTCONFIG_FILE"
      export FONTCONFIG_FILE=${fontconfig.out}/etc/fonts/fonts.conf
      #echo "FONTCONFIG_FILE=$FONTCONFIG_FILE"
    '';

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    # propagated: indentlog's exported headers (print/time.hpp) include
    # xo/timeutil headers, so downstream consumers need timeutil too
    propagatedBuildInputs = [ xo-timeutil ];

    nativeBuildInputs = [ cmake
                          catch2
                          xo-cmake ]
                        ++ lib.optionals buildDocs [
                          doxygen
                          sphinx
                          graphviz
                          fontconfig
                          python3Packages.sphinx-rtd-theme
                          python3Packages.breathe
                          python3Packages.sphinxcontrib-ditaa
                          python3Packages.sphinxcontrib-plantuml
                        ];
  })
