#{ pkgs ? import <nixpkgs> { overlays = [ (final: prev: { llvmPackages = prev.llvmPackages_17; }) ]; } }:
#pkgs.mkShell {
#  buildInputs = [ pkgs.coreutils ];
#}

{
  # official 24.05 release  # nearly works on macos, clang17, llvm18 except for sphinx-contrib.ditaa
#  nixpkgs-path ? fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-24.05.tar.gz",
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

  amf-headers-overlay = self: super: {
    amf-headers = super.amf-headers.overrideAttrs (old: {
      src = self.fetchFromGitHub {
        # nixpkgs sha256 stale for version 1.4.36 asof 4sep2025
        owner = "GPUOpen-LibrariesAndSDKs";
        repo = "AMF";
        rev = "v${old.version}";
        sha256 = "sha256-0PgWEq+329/EhI0/CgPsCkJ4CiTsFe56w2O+AcjVUdc=";
        #sha256 = "sha256-u6gvdc1acemd01TO5EbuF3H7HkEJX4GUx73xCo71yPY=";
      };});
  };

#  # nixGL not present in my nixpkgs snapshot
#  nixgl-overlay = self: super: {
#    nixGL = import (self.fetchFromGitHub {
#      owner = "nix-community";
#      repo = "nixGL";
#      rev = "main";
#      sha256 = "sha256-Ob/HuUhANoDs+nvYqyTKrkcPXf4ZgXoqMTQoCK0RFgQ=";
#    }) {pkgs = self; };
#  };

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
          xo-ratio          = self.callPackage pkgs/xo-ratio.nix          { buildDocs = true; };
          xo-unit           = self.callPackage pkgs/xo-unit.nix           { buildDocs = true; };
#
          xo-expression     = self.callPackage pkgs/xo-expression.nix     {};
          xo-tokenizer      = self.callPackage pkgs/xo-tokenizer.nix      { buildDocs = true; };
          xo-reader         = self.callPackage pkgs/xo-reader.nix         {};

          xo-jit            = self.callPackage pkgs/xo-jit.nix            { #stdenv = jitStdenv;
                                                                            #clang = llvmPackages2.clang;
                                                                            llvm = llvmPackages1.llvm; };

#
          xo-userenv-slow   = self.callPackage pkgs/xo-userenv-slow.nix   { stdenv = jitStdenv;
                                                                            #clang = llvmPackages.clang;
                                                                            llvm = llvmPackages1.llvm;
                                                                          };
          # xo-packaged version of llvm; fewer packaging features than nixpkgs
          # but easier to debug/modify
          llvmXo = llvmXo;

            xo-cmake          = self.callPackage pkgs/xo-cmake.nix          {};
            xo-indentlog      = self.callPackage pkgs/xo-indentlog.nix      { buildDocs = true; buildExamples = true; };
            xo-refcnt         = self.callPackage pkgs/xo-refcnt.nix         {};
            xo-subsys         = self.callPackage pkgs/xo-subsys.nix         {};
            xo-randomgen      = self.callPackage pkgs/xo-randomgen.nix      {                   buildExamples = true; };
            xo-ordinaltree    = self.callPackage pkgs/xo-ordinaltree.nix    {};
            xo-flatstring     = self.callPackage pkgs/xo-flatstring.nix     { buildDocs = true; buildExamples = true; };
            xo-pyutil         = self.callPackage pkgs/xo-pyutil.nix         {};
            xo-reflectutil    = self.callPackage pkgs/xo-reflectutil.nix    {};
            xo-reflect        = self.callPackage pkgs/xo-reflect.nix        {};
            xo-pyreflect      = self.callPackage pkgs/xo-pyreflect.nix      {};
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

            xo-pyexpression   = self.callPackage pkgs/xo-pyexpression.nix   {};
            xo-pyjit          = self.callPackage pkgs/xo-pyjit.nix          {};
            #
            xo-userenv        = self.callPackage pkgs/xo-userenv.nix        {};
        };

in
let
  pkgs = import nixpkgs-path {
    overlays = [
#      nixgl-overlay
      #qrencode-overlay    # not needed 4sep2025 nixpkgs (? might have src in nix store)
      #libconfig-overlay   # not needed 4sep2025 nixpkgs (? might have src in nix store)
      #pipewire-overlay    # not needed 4sep2025 nixpkgs ?
      #ccache-overlay   # not needed 4sep2025 nixpkgs
      amf-headers-overlay
#      llvm-overlay
      xo-overlay
    ];
  };
in

let
  fonts = pkgs.makeFontsConf {
    fontDirectories = [
      pkgs.inconsolata-lgc
      pkgs.noto-fonts
      pkgs.noto-fonts-emoji
      pkgs.dejavu_fonts
    ];
  };
in
pkgs.mkShell {
  # maybe should create a python environment:
  #   let
  #     pythonenv = pkgs.python3.withPackages (pset: [ pset.sphinx pset.breathe ..etc.. ])
  #   in
  #     buildInputs = [ pythonenv ..etc.. ];
  #

  buildInputs = [
    pkgs.nix

    pkgs.gsettings-desktop-schemas
    pkgs.emacs
    pkgs.nushell
    pkgs.which
    pkgs.man
    pkgs.man-pages
    pkgs.less
    pkgs.nix-tree    # needs GHC...
    pkgs.ripgrep
    pkgs.openssh
    pkgs.notmuch
    pkgs.emacsPackages.notmuch
    pkgs.inconsolata-lgc

    pkgs.python3Packages.python
    pkgs.python3Packages.pybind11
    pkgs.python3Packages.sphinx-rtd-theme
    #pkgs.python3Packages.sphinx-autobuild   # needs patch for typeguard; defer for now
    pkgs.python3Packages.breathe
    #pkgs.python3Packages.sphinxcontrib-ditaa
    pkgs.python3Packages.sphinxcontrib-plantuml
    pkgs.python3Packages.pillow

    pkgs.gdb
    pkgs.ccache
    pkgs.distcc

    pkgs.git
    pkgs.lcov
    pkgs.ditaa
    pkgs.cloc

    pkgs.sphinx
    pkgs.graphviz
    pkgs.doxygen

    pkgs.llvmPackages_18.llvm.dev
    # pkgs.llvmPackages_18.libllvm
    # pkgs.llvmPackages_18.bintools
    pkgs.libwebsockets
    pkgs.replxx
    pkgs.jsoncpp
    pkgs.eigen
    pkgs.catch2
    pkgs.zlib
    pkgs.unzip
    pkgs.libbsd

    pkgs.cmake
    pkgs.pkg-config

#    pkgs.emscripten
    pkgs.imgui
    pkgs.SDL2.dev
    pkgs.glew
#    pkgs.nixGL.nixGLDefault
#    pkgs.nixGL.nixGLNvidia
#    pkgs.nixGL.nixGLMesa
    pkgs.libGL
#    pkgs.stdenv.cc.cc.lib
#    pkgs.glibc

    # try vulkan instead of opengl (at least on OSX)
    pkgs.vulkan-loader
    pkgs.vulkan-headers
    pkgs.vulkan-validation-layers
    pkgs.darwin.moltenvk

    # fonts for imgui
#    pkgs.gucharmap
    pkgs.fontconfig
    pkgs.noto-fonts
    #pkgs.noto-fonts-lgc   # lgc for latin,greek,cyrillic   (but doesn't exist in pinned nixpkgs)
    #pkgs.noto-fonts-cjk   # cjk for chinese,japanese,korean
    pkgs.noto-fonts-emoji
    pkgs.dejavu_fonts

    pkgs.xorg.xclock
  ];

  shellHook = ''
    # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
    export SOURCE_DATE_EPOCH=$(date +%s)

    export CXENV=$CXENV:xo

    # software-only pipeline
    #export SDL_VIDEODRIVER=x11
    #export LIBGL_ALWAYS_SOFTWARE=1

    #export SDL_VIDEO_X11_FORCE_EGL=0
    export SDL_VIDEO_X11_VISUALID=0x023

    #export LIBGL_ALWAYS_INDIRECT=1
    #export WSLG_FORCE_EGL=1
    #export GLFW_USE_EGL=1

    # for Vulkan on OSX
    export VULKAN_SDK=${pkgs.vulkan-headers}
    export VK_ICD_FILENAMES=${pkgs.darwin.moltenvk}/share/vulkan/icd.d/MoltenVK_icd.json
    export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d

    # just for info
    export mesa_drivers=${pkgs.mesa.drivers}

    export FONTCONFIG_FILE=${fonts}
    export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
    export NOTO_FONTS_PATH=${pkgs.noto-fonts}/share/fonts
    export DEJAVU_FONTS_PATH=${pkgs.dejavu_fonts}/share/fonts

    # refresh font cache
    ${pkgs.fontconfig}/bin/fc-cache -fv

    echo "FONTCONFIG_PATH=$FONTCONFIG_PATH"
    echo "NOTO_FONTS_PATH=$NOTO_FONTS_PATH"

    echo "xo development environment loaded!"
  '';
}
