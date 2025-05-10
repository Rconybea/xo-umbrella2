#{ pkgs ? import <nixpkgs> { overlays = [ (final: prev: { llvmPackages = prev.llvmPackages_17; }) ]; } }:
#pkgs.mkShell {
#  buildInputs = [ pkgs.coreutils ];
#}


{
  nixpkgs-path ? ../nixpkgs,

#  pkgs ? import (fetchTarball {
#    # 24.05-darwin works on macos, clang17, llvm 18 (copying from xo-nix2)
#    url = "https://github.com/NixOS/nixpkgs/archive/dd868b7bd4d1407d607da0d1d9c5eca89132e2f7.tar.gz";
#  }),
} :

let
  # this approach (overlays) is effective, but has super wide cross-section,
  # since absolutely everything has to be rebuilt from source
  #

  llvm-overlay = self: super: {
    # use 'super' when you want to override the terms of a package.
    # use 'self'  when pointing to an existing package

    llvmPackages = super.llvmPackages_18;
  };

  xo-overlay = self: super:
    let
      # Choose the LLVM version you want
      llvmPackages = self.llvmPackages_18;
    in

      let
        # Rebuild stdenv to use that LLVM version
        customStdenv = super.overrideCC super.stdenv llvmPackages.clang;
      in

        {
          xo-cmake          = self.callPackage pkgs/xo-cmake.nix          {};
#          xo-indentlog      = self.callPackage pkgs/xo-indentlog.nix      {};
#          xo-refcnt         = self.callPackage pkgs/xo-refcnt.nix         {};
#          xo-subsys         = self.callPackage pkgs/xo-subsys.nix         {};
#          xo-randomgen      = self.callPackage pkgs/xo-randomgen.nix      {};
#          xo-ordinaltree    = self.callPackage pkgs/xo-ordinaltree.nix    {};
#          xo-pyutil         = self.callPackage pkgs/xo-pyutil.nix         {};
#          xo-flatstring     = self.callPackage pkgs/xo-flatstring.nix     {};
#          xo-reflectutil    = self.callPackage pkgs/xo-reflectutil.nix    {};
#          xo-reflect        = self.callPackage pkgs/xo-reflect.nix        {};
#          xo-pyreflect      = self.callPackage pkgs/xo-pyreflect.nix      {};
#          xo-ratio          = self.callPackage pkgs/xo-ratio.nix          {};
#          xo-unit           = self.callPackage pkgs/xo-unit.nix           {};
#          xo-pyunit         = self.callPackage pkgs/xo-pyunit.nix         {};
#          xo-expression     = self.callPackage pkgs/xo-expression.nix     {};
#          xo-pyexpression   = self.callPackage pkgs/xo-pyexpression.nix   {};
#          xo-tokenizer      = self.callPackage pkgs/xo-tokenizer.nix      {};
#          xo-reader         = self.callPackage pkgs/xo-reader.nix         {};
#          xo-jit            = self.callPackage pkgs/xo-jit.nix            { stdenv = customStdenv;
#                                                                            clang = llvmPackages.clang;
#                                                                            llvm = llvmPackages.llvm; };
#          xo-pyjit          = self.callPackage pkgs/xo-pyjit.nix          {};
#          xo-callback       = self.callPackage pkgs/xo-callback.nix       {};
#          xo-webutil        = self.callPackage pkgs/xo-webutil.nix        {};
#          xo-pywebutil      = self.callPackage pkgs/xo-pywebutil.nix      {};
#          xo-printjson      = self.callPackage pkgs/xo-printjson.nix      {};
#          xo-pyprintjson    = self.callPackage pkgs/xo-pyprintjson.nix    {};
#          xo-reactor        = self.callPackage pkgs/xo-reactor.nix        {};
#          xo-pyreactor      = self.callPackage pkgs/xo-pyreactor.nix      {};
#          xo-websock        = self.callPackage pkgs/xo-websock.nix        {};
#          xo-pywebsock      = self.callPackage pkgs/xo-pywebsock.nix      {};
#          xo-statistics     = self.callPackage pkgs/xo-statistics.nix     {};
#          xo-distribution   = self.callPackage pkgs/xo-distribution.nix   {};
#          xo-pydistribution = self.callPackage pkgs/xo-pydistribution.nix {};
#          xo-simulator      = self.callPackage pkgs/xo-simulator.nix      {};
#          xo-pysimulator    = self.callPackage pkgs/xo-pysimulator.nix    {};
#          xo-process        = self.callPackage pkgs/xo-process.nix        {};
#          xo-pyprocess      = self.callPackage pkgs/xo-pyprocess.nix      {};
#          xo-kalmanfilter   = self.callPackage pkgs/xo-kalmanfilter.nix   {};
#          xo-pykalmanfilter = self.callPackage pkgs/xo-pykalmanfilter.nix {};
#
          xo-userenv        = self.callPackage pkgs/xo-userenv.nix        {};
#          xo-userenv-slow   = self.callPackage pkgs/xo-userenv-slow.nix   {};
        };

in
let
  pkgs = import nixpkgs-path {
    overlays = [
#      llvm-overlay
      xo-overlay
    ];
  };
in
pkgs
