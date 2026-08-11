{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  # xo dependencies
  xo-facet,
  xo-cmake,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-hashable2";

    src = ../xo-hashable2;

    # NB no -DENABLE_TESTING: the facet is a scaffolded placeholder with no
    # methods and no utest/ directory.  Add the flag along with the tests.
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
      xo-facet
    ];
  })
