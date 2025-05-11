{
  # dependencies
  stdenv, cmake, #catch2,

  xo-cmake, xo-flatstring,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-reflectutil";
    version = "1.0";

    src = ../xo-reflectutil;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;
    nativeBuildInputs = [ cmake #catch2
                          xo-cmake
                          xo-flatstring
                        ];
  })
