#{ pkgs ? import <nixpkgs> { overlays = [ (final: prev: { llvmPackages = prev.llvmPackages_17; }) ]; } }:
#pkgs.mkShell {
#  buildInputs = [ pkgs.coreutils ];
#}

{
  # official 24.05 release
  #nixpkgs-path ? fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-24.05.tar.gz",
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

  # overlay to fix qrencode (very distant dependency of something pythonic)
  qrencode-overlay = self: super: {
    qrencode = super.qrencode.overrideAttrs (old: {
      nativeBuildInputs = (old.nativeBuildInputs or []) ++ [ self.autoconf self.automake self.libtool ];

      preConfigure = ''
        autoreconf -fiv
      '' + (old.preConfigure or "");

      src = self.fetchurl {
        # looks like no longer at https://fukuchi.org/works/qrencode/qrencode-4.1.1.tar.bz2
        url = "https://github.com/fukuchi/libqrencode/archive/refs/tags/v${super.qrencode.version}.tar.gz";
        sha256 = "sha256-U4W8G4wvIPO5HSWL+MzIz2ICOTXfLSZ2tbZwSfMaBJw=";
      };});
  };

  # Problem: builds *everything* with llvm18 toolchain, exposes too many compiler nits
  llvm-overlay = self: super: {
    # use 'super' when you want to override the terms of a package.
    # use 'self'  when pointing to an existing package
    #

    llvmPackages = super.llvmPackages_18;
  };

  xo-overlay = self: super:
    # use 'super' when you want to override the terms of a package.
    # use 'self'  when pointing to an existing package

    let
      # note:
      # - problems with nix llvm build-from-source w/ gcc 13.3.
      # - building outside nix following linuxfromscratch instructions, using gcc 12.3, works fine.
      #
      llvmXo = self.callPackage pkgs/llvm.nix {};

      # Choose the LLVM version you want
      llvmPackages1 = super.llvmPackages_18;

    in
      let

        # on darwin, rebuild stdenv to use clang tied to that LLVM version.
        # otherwise we get conflicts since darwin stdenv is using clang+llvm for gcc.
        #
        clangStdenv = super.overrideCC super.stdenv llvmPackages1.clang;

        # stdenv to use for xo-jit
        jitStdenv = if super.stdenv.isDarwin then clangStdenv else super.stdenv;

      in

        {
          xo-cmake          = self.callPackage pkgs/xo-cmake.nix          {};
          xo-indentlog      = self.callPackage pkgs/xo-indentlog.nix      {};
          xo-refcnt         = self.callPackage pkgs/xo-refcnt.nix         {};
          xo-subsys         = self.callPackage pkgs/xo-subsys.nix         {};
          xo-randomgen      = self.callPackage pkgs/xo-randomgen.nix      {};
          xo-ordinaltree    = self.callPackage pkgs/xo-ordinaltree.nix    {};
          xo-flatstring     = self.callPackage pkgs/xo-flatstring.nix     {};
          xo-pyutil         = self.callPackage pkgs/xo-pyutil.nix         {};
          xo-reflectutil    = self.callPackage pkgs/xo-reflectutil.nix    {};
          xo-reflect        = self.callPackage pkgs/xo-reflect.nix        {};
          xo-pyreflect      = self.callPackage pkgs/xo-pyreflect.nix      {};
          xo-ratio          = self.callPackage pkgs/xo-ratio.nix          {};
          xo-unit           = self.callPackage pkgs/xo-unit.nix           {};
          xo-pyunit         = self.callPackage pkgs/xo-pyunit.nix         {};
#
          xo-callback       = self.callPackage pkgs/xo-callback.nix       {};
          xo-webutil        = self.callPackage pkgs/xo-webutil.nix        {};
          xo-pywebutil      = self.callPackage pkgs/xo-pywebutil.nix      {};
          xo-printjson      = self.callPackage pkgs/xo-printjson.nix      {};
          xo-pyprintjson    = self.callPackage pkgs/xo-pyprintjson.nix    {};
          xo-reactor        = self.callPackage pkgs/xo-reactor.nix        {};
          xo-pyreactor      = self.callPackage pkgs/xo-pyreactor.nix      {};
          xo-websock        = self.callPackage pkgs/xo-websock.nix        {};
          xo-pywebsock      = self.callPackage pkgs/xo-pywebsock.nix      {};
#
          xo-statistics     = self.callPackage pkgs/xo-statistics.nix     {};
          xo-distribution   = self.callPackage pkgs/xo-distribution.nix   {};
          xo-pydistribution = self.callPackage pkgs/xo-pydistribution.nix {};
          xo-simulator      = self.callPackage pkgs/xo-simulator.nix      {};
          xo-pysimulator    = self.callPackage pkgs/xo-pysimulator.nix    {};
          xo-process        = self.callPackage pkgs/xo-process.nix        {};
          xo-pyprocess      = self.callPackage pkgs/xo-pyprocess.nix      {};
          xo-kalmanfilter   = self.callPackage pkgs/xo-kalmanfilter.nix   {};
          xo-pykalmanfilter = self.callPackage pkgs/xo-pykalmanfilter.nix {};
#
          xo-expression     = self.callPackage pkgs/xo-expression.nix     {};
          xo-pyexpression   = self.callPackage pkgs/xo-pyexpression.nix   {};
          xo-tokenizer      = self.callPackage pkgs/xo-tokenizer.nix      {};
          xo-reader         = self.callPackage pkgs/xo-reader.nix         {};

          xo-jit            = self.callPackage pkgs/xo-jit.nix            { #stdenv = jitStdenv;
                                                                            #clang = llvmPackages2.clang;
                                                                            llvm = llvmPackages1.llvm; };
          xo-pyjit          = self.callPackage pkgs/xo-pyjit.nix          {};

#
          xo-userenv        = self.callPackage pkgs/xo-userenv.nix        {};
          xo-userenv-slow   = self.callPackage pkgs/xo-userenv-slow.nix   { stdenv = jitStdenv;
                                                                            #clang = llvmPackages.clang;
                                                                            llvm = llvmPackages1.llvm;
                                                                          };
          # xo-packaged version of llvm; fewer packaging features than nixpkgs
          # but easier to debug/modify
          llvmXo = llvmXo;
        };

in
let
  pkgs = import nixpkgs-path {
    overlays = [
      qrencode-overlay
#      llvm-overlay
      xo-overlay
    ];
  };
in
pkgs
