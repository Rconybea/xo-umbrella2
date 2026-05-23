# xo.nix - xo package overlay, independent of nixpkgs source
# Accepts a pkgs instance (with or without non-xo overlays applied)
# Returns pkgs extended with xo packages
#
{ pkgs }:

let
  xo-overlay = self: super:
    let
      llvmXo = self.callPackage pkgs/llvm.nix {};

      llvmPackages1 = super.llvmPackages_18;

      clangStdenv = super.overrideCC super.stdenv llvmPackages1.clang;

      jitStdenv = if super.stdenv.isDarwin then clangStdenv else super.stdenv;
    in
      {
        xo-cmake          = self.callPackage pkgs/xo-cmake.nix          { stdenv = jitStdenv; };
        xo-indentlog      = self.callPackage pkgs/xo-indentlog.nix      { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
        xo-reflectutil    = self.callPackage pkgs/xo-reflectutil.nix    { stdenv = jitStdenv; };
        xo-randomgen      = self.callPackage pkgs/xo-randomgen.nix      { stdenv = jitStdenv;                   buildExamples = true; };
        xo-arena          = self.callPackage pkgs/xo-arena.nix          { stdenv = jitStdenv; buildDocs = true; };
        xo-facet          = self.callPackage pkgs/xo-facet.nix          { stdenv = jitStdenv; };
        xo-allocutil      = self.callPackage pkgs/xo-allocutil.nix      { stdenv = jitStdenv; };
        xo-refcnt         = self.callPackage pkgs/xo-refcnt.nix         { stdenv = jitStdenv; };
        xo-subsys         = self.callPackage pkgs/xo-subsys.nix         { stdenv = jitStdenv; };
        xo-testutil       = self.callPackage pkgs/xo-testutil.nix       { stdenv = jitStdenv; };
        xo-flatstring     = self.callPackage pkgs/xo-flatstring.nix     { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
        xo-pyutil         = self.callPackage pkgs/xo-pyutil.nix         { stdenv = jitStdenv; };
        xo-reflect        = self.callPackage pkgs/xo-reflect.nix        { stdenv = jitStdenv; };
        xo-pyreflect      = self.callPackage pkgs/xo-pyreflect.nix      { stdenv = jitStdenv; };
        xo-ratio          = self.callPackage pkgs/xo-ratio.nix          { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
        xo-unit           = self.callPackage pkgs/xo-unit.nix           { stdenv = jitStdenv; buildDocs = true; buildExamples = true; };
        xo-pyunit         = self.callPackage pkgs/xo-pyunit.nix         { stdenv = jitStdenv; };
        xo-callback       = self.callPackage pkgs/xo-callback.nix       { stdenv = jitStdenv; };
        xo-printable2     = self.callPackage pkgs/xo-printable2.nix     { stdenv = jitStdenv; };
        xo-alloc          = self.callPackage pkgs/xo-alloc.nix          { stdenv = jitStdenv; buildDocs = true; };
        xo-alloc2         = self.callPackage pkgs/xo-alloc2.nix         { stdenv = jitStdenv; buildDocs = true; };
        xo-stringtable2   = self.callPackage pkgs/xo-stringtable2.nix   { stdenv = jitStdenv; };
        xo-gc             = self.callPackage pkgs/xo-gc.nix             { stdenv = jitStdenv; buildDocs = true; };
        xo-object         = self.callPackage pkgs/xo-object.nix         { stdenv = jitStdenv; };
        xo-type           = self.callPackage pkgs/xo-type.nix           { stdenv = jitStdenv; };
        xo-object2        = self.callPackage pkgs/xo-object2.nix        { stdenv = jitStdenv; };
        xo-procedure2     = self.callPackage pkgs/xo-procedure2.nix     { stdenv = jitStdenv; };
        xo-numeric        = self.callPackage pkgs/xo-numeric.nix        { stdenv = jitStdenv; };
        xo-ordinaltree    = self.callPackage pkgs/xo-ordinaltree.nix    { stdenv = jitStdenv; };
        xo-tokenizer2     = self.callPackage pkgs/xo-tokenizer2.nix     { stdenv = jitStdenv;                   buildExamples = true; };
        xo-webutil        = self.callPackage pkgs/xo-webutil.nix        {};
        xo-pywebutil      = self.callPackage pkgs/xo-pywebutil.nix      {};
        xo-printjson      = self.callPackage pkgs/xo-printjson.nix      {};
        xo-pyprintjson    = self.callPackage pkgs/xo-pyprintjson.nix    {};
        xo-reactor        = self.callPackage pkgs/xo-reactor.nix        {};
        xo-pyreactor      = self.callPackage pkgs/xo-pyreactor.nix      {};
        xo-websock        = self.callPackage pkgs/xo-websock.nix        {};
        xo-pywebsock      = self.callPackage pkgs/xo-pywebsock.nix      {};
        xo-statistics     = self.callPackage pkgs/xo-statistics.nix     {};
        xo-distribution   = self.callPackage pkgs/xo-distribution.nix   {};
        xo-pydistribution = self.callPackage pkgs/xo-pydistribution.nix {};
        xo-simulator      = self.callPackage pkgs/xo-simulator.nix      {};
        xo-pysimulator    = self.callPackage pkgs/xo-pysimulator.nix    {};
        xo-process        = self.callPackage pkgs/xo-process.nix        {};
        xo-pyprocess      = self.callPackage pkgs/xo-pyprocess.nix      {};
        xo-kalmanfilter   = self.callPackage pkgs/xo-kalmanfilter.nix   {};
        xo-pykalmanfilter = self.callPackage pkgs/xo-pykalmanfilter.nix {};
        xo-expression     = self.callPackage pkgs/xo-expression.nix     {                   buildExamples = true; };
        xo-pyexpression   = self.callPackage pkgs/xo-pyexpression.nix   {};
        xo-tokenizer      = self.callPackage pkgs/xo-tokenizer.nix      { buildDocs = true; buildExamples = true; };
        xo-reader         = self.callPackage pkgs/xo-reader.nix         { buildDocs = true; buildExamples = true; };
        xo-jit            = self.callPackage pkgs/xo-jit.nix            { buildDocs = true; buildExamples = true;
                                                                          llvm = llvmPackages1.llvm; };
        xo-pyjit          = self.callPackage pkgs/xo-pyjit.nix          {};
        xo-userenv        = self.callPackage pkgs/xo-userenv.nix        {};
        xo-userenv-slow   = self.callPackage pkgs/xo-userenv-slow.nix   { stdenv = jitStdenv;
                                                                          llvm = llvmPackages1.llvm; };
        llvmXo = llvmXo;
      };

  # applies overlay to already-constructed package collection
  xoPkgs = pkgs.extend xo-overlay;

in
xoPkgs
