{
  # dependencies
  stdenv, cmake, catch2, doxygen,

  python3Packages,
  sphinx,

  xo-cmake, xo-ratio, xo-flatstring,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-unit";
    version = "1.0";

    src = ../xo-unit;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    propagatedBuildInputs = [ xo-ratio xo-flatstring ];
    nativeBuildInputs = [ cmake
                          catch2
                          doxygen
                          sphinx
                          xo-cmake
                          #python3Packages.sphinx-rtd-theme
                        ];
  })
