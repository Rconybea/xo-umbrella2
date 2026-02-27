{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,
  doxygen,

  python3Packages,

  sphinx, graphviz,

  # xo dependencies
  xo-callback,
  xo-unit,
  xo-reflect,
#  xo-randomgen,
  xo-reflectutil,
  xo-indentlog,
  xo-allocutil,
  xo-cmake,

  buildDocs ? false,
  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-alloc";

    src = ../xo-alloc;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals buildDocs ["-DXO_ENABLE_DOCS=on"]
                  ++ ["-DENABLE_TESTING=1"];

    inherit buildDocs;
    inherit doCheck;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    nativeBuildInputs = [
      cmake
      catch2
      xo-cmake
#      xo-randomgen
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
      xo-callback
      xo-unit
      xo-reflect
      xo-allocutil
      xo-reflectutil
      xo-indentlog
    ];
  })
