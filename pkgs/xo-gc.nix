{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,
  doxygen,

  python3Packages,

  sphinx, graphviz,

  # xo dependencies
  xo-alloc2,
  xo-facet,
  xo-randomgen,
  xo-subsys,
#  xo-reflectutil,
#  xo-indentlog,
  xo-cmake,

  buildDocs ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-gc";

    src = ../xo-gc;

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
      xo-cmake
      xo-randomgen
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
      xo-alloc2
      xo-facet
      xo-subsys
#      xo-reflectutil
#      xo-indentlog
    ];
  })
