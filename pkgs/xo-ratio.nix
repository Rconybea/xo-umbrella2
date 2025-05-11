{
  # dependencies
  stdenv, cmake, #catch2,
  doxygen,

  python3Packages,

  sphinx,

  xo-cmake, xo-flatstring, xo-reflectutil,
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
