{
  # dependencies
  lib, stdenv, cmake, #catch2,

  python3Packages,

  doxygen, sphinx, graphviz,

  xo-cmake, xo-flatstring, xo-reflectutil,

  buildDocs ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-ratio";
    version = "1.0";

    src = ../xo-ratio;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    inherit buildDocs;
    doCheck = true;

    postBuild = lib.optionalString buildDocs ''
      cmake --build . -- docs
    '';

    propagatedBuildInputs = [ ];

    nativeBuildInputs = [
      cmake
      xo-cmake
      xo-flatstring
      xo-reflectutil
    ]
    ++ lib.optionals buildDocs [
      doxygen
      sphinx
      graphviz
      python3Packages.sphinx-rtd-theme
      python3Packages.breathe
      python3Packages.sphinxcontrib-ditaa
    ];
  })
