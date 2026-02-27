{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,
  doxygen,

  python3Packages,

  sphinx, graphviz,

  # xo dependencies
  xo-facet,
#  xo-randomgen,
#  xo-reflectutil,
#  xo-indentlog,
  xo-cmake,

#  buildDocs ? false,
#  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-printable2";

    src = ../xo-printable2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
#                  ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
#                  ++ ["-DENABLE_TESTING=1"];
      ;

#    inherit buildDocs;
#    inherit doCheck;

#    postBuild = lib.optionalString buildDocs ''
#      cmake --build . -- docs
#    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
#      xo-randomgen
#    ] ++ lib.optionals buildDocs [
#      doxygen
#      sphinx
#      graphviz
#      python3Packages.sphinx-rtd-theme
#      python3Packages.breathe
#      python3Packages.sphinxcontrib-ditaa
#      python3Packages.sphinxcontrib-plantuml
#      python3Packages.pillow
    ];
    propagatedBuildInputs = [
      xo-facet
#      xo-reflectutil
#      xo-indentlog
    ];
  })
