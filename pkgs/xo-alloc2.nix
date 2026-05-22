{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,
  doxygen, cli11,

  python3Packages,

  sphinx, graphviz,

  # xo dependencies
  xo-facet,
  xo-randomgen,
  xo-arena,
  xo-subsys,
  xo-testutil,
#  xo-callback,
#  xo-unit,
#  xo-reflect,
#  xo-randomgen,
#  xo-reflectutil,
#  xo-indentlog,
#  xo-allocutil,
  xo-cmake,

  buildDocs ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-alloc2";

    src = ../xo-alloc2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit doCheck;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [
      cmake
      catch2
      cli11
      xo-randomgen
      xo-cmake
    ] ++ lib.optionals doCheck [
      xo-testutil
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
      xo-arena
      xo-facet
      xo-subsys
#      xo-callback
#      xo-unit
#      xo-reflect
#      xo-allocutil
#      xo-reflectutil
#      xo-indentlog
    ];
  })
