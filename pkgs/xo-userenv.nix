# builds environment with all xo packages,
# using combined output directory for each.
#
# parallels github actions build on stock ubuntu,
# except that we use nixpkgs for toolchain
#
# For xo development, probably prefer xo-userenv.nix instead of this xo-userenv-slow.nix:
# 1. xo-userenv.nix allows parallel build
# 2. xo-userenv.nix only rebuilds xo packages that have changed

{
  # nixpkgs dependencies
  buildEnv,
  stdenv,
  cmake,
  catch2,
  eigen,
  libwebsockets,
  jsoncpp,
  doxygen,
  sphinx,
  python3Packages,

  # xo dependencies
  xo-cmake,
  xo-indentlog,
  xo-refcnt,
  xo-subsys,
  xo-randomgen,
  xo-ordinaltree,
  xo-flatstring,
  xo-pyutil,
  xo-reflectutil,
  xo-reflect,
  xo-pyreflect,
  xo-ratio,
  xo-unit,
  xo-pyunit,
#
  xo-callback,
  xo-webutil,
  xo-pywebutil,
#  xo-printjson,
#  xo-pyprintjson,
#  xo-reactor,
#  xo-pyreactor,
#  xo-simulator,
#  xo-pysimulator, xo-distribution, xo-pydistribution, xo-process, xo-pyprocess, xo-statistics, xo-kalmanfilter,
#  xo-pykalmanfilter, xo-websock, xo-pywebsock,

  xo-expression,
  xo-pyexpression,
  xo-tokenizer,
  xo-reader,
  xo-jit,
  xo-pyjit

  # other args

  # someconfigurationoption ? false
} :

buildEnv {
  name = "xo-userenv";
  paths = [ xo-cmake
            xo-indentlog
            xo-refcnt
            xo-subsys
            xo-randomgen
            xo-ordinaltree
            xo-flatstring
            xo-pyutil
            xo-reflectutil
            xo-reflect
            xo-pyreflect
            xo-ratio
            xo-unit
            xo-pyunit
#
            xo-callback
            xo-webutil
            xo-pywebutil
#
            xo-expression
            xo-pyexpression
            xo-tokenizer
            xo-reader
            xo-jit
            xo-pyjit
#            xo-printjson
#            xo-pyprintjson
#            xo-reactor
#            xo-pyreactor
#            xo-websock
#            xo-pywebsock
#            xo-statistics
#            xo-distribution
#            xo-pydistribution
#            xo-simulator
#            xo-pysimulator
#            xo-process
#            xo-pyprocess
#            xo-kalmanfilter
#            xo-pykalmanfilter
          ];
}
