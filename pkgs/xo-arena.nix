{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,
  doxygen,

  python3Packages,

  sphinx, graphviz,

  # xo dependencies
  xo-randomgen,
  xo-reflectutil,
  xo-indentlog,
  xo-cmake,

  doCheck ? true,
  buildDocs ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-arena";

    src = ../xo-arena;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit doCheck;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake xo-randomgen
    ] ++ lib.optionals buildDocs [
      doxygen
      sphinx
      graphviz
      python3Packages.sphinx-rtd-theme
      python3Packages.breathe
      python3Packages.sphinxcontrib-ditaa
      python3Packages.sphinxcontrib-plantuml
      python3Packages.pillow
    ];
    propagatedBuildInputs = [
      xo-reflectutil
      xo-indentlog
    ];
  })
