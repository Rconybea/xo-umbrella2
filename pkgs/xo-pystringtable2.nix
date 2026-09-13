{
  # nixpkgs dependencies
  lib, stdenv, cmake, catch2,

  python3Packages,

  # xo dependencies
  xo-cmake,
  xo-pyutil,
  xo-refcnt,
  xo-stringtable2,
  # FacetAppcx is configure()'s second argument; xo-pyfacet registers it with
  # pybind11 and this module imports that at init
  xo-facet, xo-pyfacet,
  # String's bindings: APrintable (the facet its handle is keyed on),
  # IAllocator_DArena (obj<AAllocator> over the flywheel's arena), and PpSink
  # (String.pretty's argument type)
  xo-printable2, xo-alloc2, xo-ppsink,
  # MemorySizeInfo, the element type visit_pools() returns
  xo-arena, xo-pyarena,
  # PpSink is REGISTERED by xo-pyindentlog2, which this module imports at init;
  # xo.facet's configure_all() reaches xo.indentlog2 as well
  xo-pyindentlog2,

  doCheck ? true,
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-pystringtable2";
    version = "1.0";

    src = ../xo-pystringtable2;

    # ENABLE_TESTING alongside doCheck, not doCheck alone: utest/CMakeLists.txt
    # registers the python tests inside `if (ENABLE_TESTING)', so the flag is
    # what makes ctest find them rather than run over an empty set and report
    # success.
    cmakeFlags = ["-DCMAKE_MODULE_PATH=${xo-cmake}/share/cmake"]
                 ++ lib.optionals doCheck ["-DENABLE_TESTING=1"];

    inherit doCheck;

    # The tests import xo.facet and xo.indentlog2 as well as xo.stringtable2,
    # and only xo.stringtable2 is built here.  This composes because `xo' is a
    # PEP 420 namespace package (.xo-backlog/python-packaging/issues/01): each
    # store path holds an xo/ with one module in it, and python merges them
    # rather than letting the first shadow the rest.
    preCheck = ''
      export PYTHONPATH=${xo-pyfacet}/lib/python:${xo-pyindentlog2}/lib/python:${xo-pyarena}/lib/python
    '';

    nativeBuildInputs = [
      cmake catch2
      xo-cmake
      python3Packages.pybind11
    ];
    propagatedBuildInputs = [
      xo-pyutil
      xo-refcnt
      xo-stringtable2
      xo-facet
      xo-pyfacet
      xo-printable2
      xo-alloc2
      xo-ppsink
      xo-arena
      xo-pyarena
      xo-pyindentlog2
    ];
  })
