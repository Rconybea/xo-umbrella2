{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake,
  xo-pyutil,
  xo-reactor2websock,
  # imported at module init: xo.reactor and xo.webutil
  xo-pyreactor,
  xo-pywebutil,
  # imported, in turn, by xo.reactor (xo.printjson, xo.reflect) and by
  # xo.reflect (xo.indentlog2)
  xo-pyprintjson,
  xo-pyreflect,
  xo-pyindentlog2,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pyreactor2websock";
    version = "1.0";

    src = ../xo-pyreactor2websock;

    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];
    inherit doCheck;

    # The smoke test imports xo.reactor2websock, whose init imports the chain
    # above.  Each of those modules lives in its own store path; `xo' is a PEP
    # 420 namespace package, so python merges them -- see pkgs/xo-pyobject2.nix.
    preCheck = ''
      export PYTHONPATH=${xo-pyreactor}/lib/python:${xo-pywebutil}/lib/python:${xo-pyprintjson}/lib/python:${xo-pyreflect}/lib/python:${xo-pyindentlog2}/lib/python
    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-reactor2websock
      xo-pyreactor
      xo-pywebutil
      xo-pyprintjson
      xo-pyreflect
      xo-pyindentlog2
    ];
  })
