{
  # dependencies

  stdenv,
  cmake
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-cmake";

    src = ../xo-cmake;

    nativeBuildInputs = [ cmake ];
  })
