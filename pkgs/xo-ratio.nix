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
    doCheck = true;
    nativeBuildInputs = [ cmake
                          #catch2
                          doxygen
                          sphinx
#                          python3Packages.sphinx-rtd-theme
                          xo-cmake
                          xo-flatstring
                          xo-reflectutil
                        ];
  })
