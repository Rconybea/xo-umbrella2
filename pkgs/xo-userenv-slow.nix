
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
  stdenv,
  cmake,
  catch2,
  eigen,
  libwebsockets,
  jsoncpp,
  doxygen,
  sphinx,
  python3Packages,
  llvm,
  which,

  # xo dependencies
  xo-cmake, xo-indentlog, xo-subsys, xo-refcnt, xo-randomgen, xo-ordinaltree, xo-flatstring, xo-reflectutil,
  xo-ratio, xo-pyutil,
  xo-reflect, xo-pyreflect,
  xo-unit, xo-pyunit,
  xo-printjson, xo-pyprintjson, xo-callback, xo-webutil, xo-pywebutil, xo-reactor, xo-pyreactor, xo-simulator,
  xo-pysimulator, xo-distribution, xo-pydistribution, xo-process, xo-pyprocess, xo-statistics, xo-kalmanfilter,
  xo-pykalmanfilter, xo-websock, xo-pywebsock, xo-tokenizer, xo-expression, xo-pyexpression, xo-reader,
  xo-jit, xo-pyjit,

  # other args

  # someconfigurationoption ? false
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-userenv";

    xo_cmake_src          = xo-cmake.src;
    xo_indentlog_src      = xo-indentlog.src;
    xo_subsys_src         = xo-subsys.src;
    xo_refcnt_src         = xo-refcnt.src;
    xo_randomgen_src      = xo-randomgen.src;
    xo_ordinaltree_src    = xo-ordinaltree.src;
    xo_flatstring_src     = xo-flatstring.src;
    xo_reflectutil_src    = xo-reflectutil.src;
    xo_ratio_src          = xo-ratio.src;
    xo_pyutil_src         = xo-pyutil.src;
    xo_reflect_src        = xo-reflect.src;
    xo_pyreflect_src      = xo-pyreflect.src;
    xo_unit_src           = xo-unit.src;
    xo_pyunit_src         = xo-pyunit.src;
    xo_printjson_src      = xo-printjson.src;
    xo_pyprintjson_src    = xo-pyprintjson.src;
    xo_callback_src       = xo-callback.src;
    xo_webutil_src        = xo-webutil.src;
    xo_pywebutil_src      = xo-pywebutil.src;
    xo_reactor_src        = xo-reactor.src;
    xo_pyreactor_src      = xo-pyreactor.src;
    xo_simulator_src      = xo-simulator.src;
    xo_pysimulator_src    = xo-pysimulator.src;
    xo_distribution_src   = xo-distribution.src;
    xo_pydistribution_src = xo-pydistribution.src;
    xo_process_src        = xo-process.src;
    xo_pyprocess_src      = xo-pyprocess.src;
    xo_statistics_src     = xo-statistics.src;
    xo_kalmanfilter_src   = xo-kalmanfilter.src;
    xo_pykalmanfilter_src = xo-pykalmanfilter.src;
    xo_websock_src        = xo-websock.src;
    xo_pywebsock_src      = xo-pywebsock.src;
    xo_tokenizer_src      = xo-tokenizer.src;
    xo_expression_src     = xo-expression.src;
    xo_pyexpression_src   = xo-pyexpression.src;
    xo_reader_src         = xo-reader.src;
    xo_jit_src            = xo-jit.src;
    xo_pyjit_src          = xo-pyjit.src;

    # instead of this probably want to override phases.
    buildCommand = ''

    mkdir $out
    PREFIX=$out
    PATH=$out/bin:$PATH

    mkdir -p build/xo-cmake

    cmake -DCMAKE_INSTALL_PREFIX=$out -B build/xo-cmake -S $xo_cmake_src
    cmake --build build/xo-cmake
    cmake --install build/xo-cmake

    bash=$(which bash)
    xobuild=$(which xo-build)
    xoconfig=$(which xo-cmake-config)

    sed -i -e "1s:/usr/bin/env bash:$bash:" $xobuild
    sed -i -e "1s:/usr/bin/env bash:$bash:" $xoconfig
    # xo-build doesn't support the pattern here, maybe fix it
    xo-build --configure -S $xo_indentlog_src      -B build/xo-indentlog      --build --install xo-indentlog
    xo-build --configure -S $xo_subsys_src         -B build/xo-subsys         --build --install xo-subsys
    xo-build --configure -S $xo_refcnt_src         -B build/xo-refcnt         --build --install xo-refcnt
    xo-build --configure -S $xo_randomgen_src      -B build/xo-randomgen      --build --install xo-randomgen
    xo-build --configure -S $xo_ordinaltree_src    -B build/xo-ordinaltree    --build --install xo-randomgen
    xo-build --configure -S $xo_flatstring_src     -B build/xo-flatstring     --build --install xo-flatstring
    xo-build --configure -S $xo_reflectutil_src    -B build/xo-reflectutil    --build --install xo-reflectutil
    xo-build --configure -S $xo_ratio_src          -B build/xo-ratio          --build --install xo-ratio
    xo-build --configure -S $xo_pyutil_src         -B build/xo-pyutil         --build --install xo-pyutil
    xo-build --configure -S $xo_reflect_src        -B build/xo-reflect        --build --install xo-reflect
    xo-build --configure -S $xo_pyreflect_src      -B build/xo-pyreflect      --build --install xo-pyreflect
    xo-build --configure -S $xo_unit_src           -B build/xo-unit           --build --install xo-unit
    xo-build --configure -S $xo_pyunit_src         -B build/xo-pyunit         --build --install xo-pyunit
    xo-build --configure -S $xo_printjson_src      -B build/xo-printjson      --build --install xo-printjson
    xo-build --configure -S $xo_pyprintjson_src    -B build/xo-pyprintjson    --build --install xo-pyprintjson
    xo-build --configure -S $xo_callback_src       -B build/xo-callback       --build --install xo-callback
    xo-build --configure -S $xo_webutil_src        -B build/xo-webutil        --build --install xo-webutil
    xo-build --configure -S $xo_pywebutil_src      -B build/xo-pywebutil      --build --install xo-pywebutil
    xo-build --configure -S $xo_reactor_src        -B build/xo-reactor        --build --install xo-reactor
    xo-build --configure -S $xo_pyreactor_src      -B build/xo-pyreactor      --build --install xo-pyreactor
    xo-build --configure -S $xo_simulator_src      -B build/xo-simulator      --build --install xo-simulator
    xo-build --configure -S $xo_pysimulator_src    -B build/xo-pysimulator    --build --install xo-pysimulator
    xo-build --configure -S $xo_distribution_src   -B build/xo-distribution   --build --install xo-distribution
    xo-build --configure -S $xo_pydistribution_src -B build/xo-pydistribution --build --install xo-pydistribution
    xo-build --configure -S $xo_process_src        -B build/xo-process        --build --install xo-process
    xo-build --configure -S $xo_pyprocess_src      -B build/xo-pyprocess      --build --install xo-pyprocess
    xo-build --configure -S $xo_statistics_src     -B build/xo-statistics     --build --install xo-statistics
    xo-build --configure -S $xo_kalmanfilter_src   -B build/xo-kalmanfilter   --build --install xo-kalmanfilter
    xo-build --configure -S $xo_pykalmanfilter_src -B build/xo-pykalmanfilter --build --install xo-pykalmanfilter
    xo-build --configure -S $xo_websock_src        -B build/xo-websock        --build --install xo-websock
    xo-build --configure -S $xo_pywebsock_src      -B build/xo-pywebsock      --build --install xo-pywebsock
    xo-build --configure -S $xo_tokenizer_src      -B build/xo-tokenizer      --build --install xo-tokenizer
    xo-build --configure -S $xo_expression_src     -B build/xo-expression     --build --install xo-expression
    xo-build --configure -S $xo_pyexpression_src   -B build/xo-pyexpression   --build --install xo-pyexpression
    xo-build --configure -S $xo_reader_src         -B build/xo-reader         --build --install xo-reader
    xo-build --configure -S $xo_jit_src            -B build/xo-jit            --build --install xo-jit
    xo-build --configure -S $xo_pyjit_src          -B build/xo-pyjit          --build --install xo-pyjit

'';

    nativeBuildInputs = [ cmake catch2 eigen libwebsockets jsoncpp doxygen sphinx which
                          llvm.dev
                          python3Packages.pybind11
                          python3Packages.sphinx-rtd-theme
                        ];

  # runScript = ...;
  # profile = ...;
  })
