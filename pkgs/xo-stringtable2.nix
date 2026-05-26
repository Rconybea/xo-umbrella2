{
  # nixpkgs dependencies
  stdenv, cmake, catch2,

  # xo dependencies
  xo-alloc2,
  xo-printable2,
  xo-arena,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-stringtable2";

    src = ../xo-stringtable2;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"
                  "-DENABLE_TESTING=1"
                 ];
    doCheck = true;
    nativeBuildInputs = [
      cmake catch2 xo-cmake
    ];
    propagatedBuildInputs = [
      xo-alloc2
      xo-printable2
      xo-arena
    ];
  })
