# xo.nix - xo package set, independent of nixpkgs source
# Accepts a pkgs instance (with or without non-xo overlays applied)
# Returns attrset of xo packages.
#
# Callers decide how to consume:
#   ci.nix      - uses as plain attrset
#   default.nix - wraps as overlay for dev shell
#
{ pkgs }:

let
  inherit (pkgs) lib stdenv;

  llvmXo = lib.callPackageWith (pkgs // xoPackages) pkgs/llvm.nix {};

  llvmPackages1 = pkgs.llvmPackages_18;

  clangStdenv = pkgs.overrideCC stdenv llvmPackages1.clang;

  # stdenv to use for xo-jit and friends
  jitStdenv = if stdenv.isDarwin then clangStdenv else stdenv;

  # callPackageWith sees both nixpkgs AND xo packages,
  # so xo-alloc2 can resolve xo-arena etc.
  callPackage = lib.callPackageWith (pkgs // xoPackages);

  xoPackages = {
    xo-cmake          = callPackage pkgs/xo-cmake.nix          { stdenv = jitStdenv; };
    xo-indentlog      = callPackage pkgs/xo-indentlog.nix      { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-subsys         = callPackage pkgs/xo-subsys.nix         { stdenv = jitStdenv; };
    xo-flatstring     = callPackage pkgs/xo-flatstring.nix     { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-randomgen      = callPackage pkgs/xo-randomgen.nix      { stdenv = jitStdenv;                   buildExamples = true; };
    xo-reflectutil    = callPackage pkgs/xo-reflectutil.nix    { stdenv = jitStdenv; };
    xo-arena          = callPackage pkgs/xo-arena.nix          { stdenv = jitStdenv; buildDocs = true; };
    xo-facet          = callPackage pkgs/xo-facet.nix          { stdenv = jitStdenv; };
    xo-printable2     = callPackage pkgs/xo-printable2.nix     { stdenv = jitStdenv; };
    xo-testutil       = callPackage pkgs/xo-testutil.nix       { stdenv = jitStdenv; };
    xo-alloc2         = callPackage pkgs/xo-alloc2.nix         { stdenv = jitStdenv; buildDocs = true; };
    xo-refcnt         = callPackage pkgs/xo-refcnt.nix         { stdenv = jitStdenv; };
    xo-pyutil         = callPackage pkgs/xo-pyutil.nix         { stdenv = jitStdenv; };
    xo-reflect        = callPackage pkgs/xo-reflect.nix        { stdenv = jitStdenv; };
    xo-pyreflect      = callPackage pkgs/xo-pyreflect.nix      { stdenv = jitStdenv; };
    xo-stringtable2   = callPackage pkgs/xo-stringtable2.nix   { stdenv = jitStdenv; };
    xo-object2        = callPackage pkgs/xo-object2.nix        { stdenv = jitStdenv; };
    xo-gc             = callPackage pkgs/xo-gc.nix             { stdenv = jitStdenv; buildDocs = true; };
    xo-type           = callPackage pkgs/xo-type.nix           { stdenv = jitStdenv; };
    xo-procedure2     = callPackage pkgs/xo-procedure2.nix     { stdenv = jitStdenv; };
    xo-numeric        = callPackage pkgs/xo-numeric.nix        { stdenv = jitStdenv; };
    xo-tokenizer2     = callPackage pkgs/xo-tokenizer2.nix     { stdenv = jitStdenv;                   buildExamples = true; };
    xo-expression2    = callPackage pkgs/xo-expression2.nix    { stdenv = jitStdenv; };
    xo-reader2        = callPackage pkgs/xo-reader2.nix        { stdenv = jitStdenv; };
    xo-interpreter2   = callPackage pkgs/xo-interpreter2.nix   { stdenv = jitStdenv; };
    xo-allocutil      = callPackage pkgs/xo-allocutil.nix      { stdenv = jitStdenv; };
    xo-alloc          = callPackage pkgs/xo-alloc.nix          { stdenv = jitStdenv; buildDocs = true; };
    xo-ratio          = callPackage pkgs/xo-ratio.nix          { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-unit           = callPackage pkgs/xo-unit.nix           { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-pyunit         = callPackage pkgs/xo-pyunit.nix         { stdenv = jitStdenv; };
    xo-callback       = callPackage pkgs/xo-callback.nix       { stdenv = jitStdenv; };
    xo-object         = callPackage pkgs/xo-object.nix         { stdenv = jitStdenv; };
    xo-ordinaltree    = callPackage pkgs/xo-ordinaltree.nix    { stdenv = jitStdenv; };
    xo-expression     = callPackage pkgs/xo-expression.nix     { stdenv = jitStdenv;                   buildExamples = true; };
    xo-pyexpression   = callPackage pkgs/xo-pyexpression.nix   { stdenv = jitStdenv; };
    xo-tokenizer      = callPackage pkgs/xo-tokenizer.nix      { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-reader         = callPackage pkgs/xo-reader.nix         { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
    xo-jit            = callPackage pkgs/xo-jit.nix            { stdenv = jitStdenv; buildDocs = true; buildExamples = true; llvm = llvmPackages1.llvm; };
    xo-webutil        = callPackage pkgs/xo-webutil.nix        { stdenv = jitStdenv; };
    xo-pywebutil      = callPackage pkgs/xo-pywebutil.nix      {};
    xo-printjson      = callPackage pkgs/xo-printjson.nix      {};
    xo-pyprintjson    = callPackage pkgs/xo-pyprintjson.nix    {};
    xo-reactor        = callPackage pkgs/xo-reactor.nix        {};
    xo-pyreactor      = callPackage pkgs/xo-pyreactor.nix      {};
    xo-websock        = callPackage pkgs/xo-websock.nix        {};
    xo-pywebsock      = callPackage pkgs/xo-pywebsock.nix      {};
    xo-statistics     = callPackage pkgs/xo-statistics.nix     {};
    xo-distribution   = callPackage pkgs/xo-distribution.nix   {};
    xo-pydistribution = callPackage pkgs/xo-pydistribution.nix {};
    xo-simulator      = callPackage pkgs/xo-simulator.nix      {};
    xo-pysimulator    = callPackage pkgs/xo-pysimulator.nix    {};
    xo-process        = callPackage pkgs/xo-process.nix        {};
    xo-pyprocess      = callPackage pkgs/xo-pyprocess.nix      {};
    xo-kalmanfilter   = callPackage pkgs/xo-kalmanfilter.nix   {};
    xo-pykalmanfilter = callPackage pkgs/xo-pykalmanfilter.nix {};
    xo-pyjit          = callPackage pkgs/xo-pyjit.nix          {};
    xo-userenv        = callPackage pkgs/xo-userenv.nix        {};
    xo-userenv-slow   = callPackage pkgs/xo-userenv-slow.nix   { stdenv = jitStdenv;
                                                                  llvm = llvmPackages1.llvm; };
    inherit llvmXo;
  };

in xoPackages
