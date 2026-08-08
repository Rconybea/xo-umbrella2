{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2, replxx,

  # xo dependencies
  xo-cmake,
  xo-object,
  xo-expression,
  xo-reader,
  xo-ppsink,
  xo-indentlog2,
  xo-subsys,

  # config arguments
  buildExamples ? true,
  doCheck ? true
} :

stdenv.mkDerivation (finalattrs :
  {
    name = "xo-interpreter";

    src = ../xo-interpreter;

    # examples on by default: example/replxx/ is the only consumer of this
    # subsystem, so it is the only end-to-end check that the installed
    # package is usable.  (Also how the xo-tokenizer example regression was
    # caught -- see .xo-backlog/xo-imgui/issues/01.)
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                  ++ lib.optionals doCheck ["-DENABLE_TESTING=1"]
                  ++ lib.optionals buildExamples ["-DXO_ENABLE_EXAMPLES=1"];

    inherit doCheck;

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
    ];
    propagatedBuildInputs = [
      # replxx is PUBLIC on the library target (replxx::replxx appears in its
      # INTERFACE_LINK_LIBRARIES), so it must be propagated, not native-only.
      replxx
      xo-object
      xo-expression
      xo-reader
      xo-ppsink
      xo-indentlog2
      xo-subsys
    ];
  })
