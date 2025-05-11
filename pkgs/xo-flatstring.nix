{
  # dependencies
  lib, stdenv, cmake, catch2, doxygen,

  python3Packages,

  sphinx,

  xo-cmake, xo-indentlog,

  buildDocs ? false,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-flatstring";
    version = "1.0";

    outputs = [
      "out"
    ]
#    ++ lib.optionals buildDocs [
#      "doc"
    #    ]
    ;

    src = ../xo-flatstring;

    # guessing may need this for doxygen etc
    # env.LC_ALL = "en_US.UTF-8";

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"];
    doCheck = true;

    propagatedBuildInputs = [ ];

    nativeBuildInputs = [
      cmake
      catch2
      doxygen
      sphinx
      xo-cmake
      xo-indentlog
    ]
#    + lib.optionals buildDocs [
#      doxygen
#      sphinx
#      python3Packages.sphinx-rtd-theme
#      python3Packages.breathe
    #    ]
    ;

  })
