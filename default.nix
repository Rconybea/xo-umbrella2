#{ pkgs ? import <nixpkgs> { overlays = [ (final: prev: { llvmPackages = prev.llvmPackages_17; }) ]; } }:
#pkgs.mkShell {
#  buildInputs = [ pkgs.coreutils ];
#}

{
  # official 24.05 release  # nearly works on macos, clang17, llvm18 except for sphinx-contrib.ditaa
# probably whould be nixos-25.05.tar.gz here
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

  # complains about 'no more ptys'
  dejagnu-overlay = self: super: {
    dejagnu = super.dejagnu.overrideAttrs (old: {
      doCheck = false;
    });
  };

  swtpm-overlay = self: super: {
    swtpm = super.swtpm.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # libffi tests run, but only if sandbox disabled
  libffi-overlay = self: super: {
    libffi = super.libffi.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # libqmi test seems to stall on ubuntu
  libqmi-overlay = self: super: {
    libqmi = super.libqmi.overrideAttrs (old: {
      doCheck = false;
    });
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

  # tests excruciatingly slow
  mailutils-overlay = self: super: {
    mailutils = super.mailutils.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # 2 tests fail with 25.05
  notmuch-overlay = self: super: {
    notmuch = super.notmuch.overrideAttrs (old: {
      doCheck = false;
    });
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
            xo-ratio          = self.callPackage pkgs/xo-ratio.nix          { buildDocs = true; buildExamples = true; };
            xo-unit           = self.callPackage pkgs/xo-unit.nix           { buildDocs = true; buildExamples = true; };
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

            xo-expression     = self.callPackage pkgs/xo-expression.nix     {                   buildExamples = true; };
            xo-pyexpression   = self.callPackage pkgs/xo-pyexpression.nix   {};
            xo-tokenizer      = self.callPackage pkgs/xo-tokenizer.nix      { buildDocs = true; buildExamples = true; };
            xo-reader         = self.callPackage pkgs/xo-reader.nix         { buildDocs = true; buildExamples = true; };

            xo-jit            = self.callPackage pkgs/xo-jit.nix            { buildDocs = true; buildExamples = true;
                                                                              #stdenv = jitStdenv;
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
      #qrencode-overlay    # not needed 4sep2025 nixpkgs (? might have src in nix store)
      #libconfig-overlay   # not needed 4sep2025 nixpkgs (? might have src in nix store)
      #pipewire-overlay    # not needed 4sep2025 nixpkgs ?
      #ccache-overlay   # not needed 4sep2025 nixpkgs
      amf-headers-overlay
      dejagnu-overlay
      libffi-overlay
      libqmi-overlay
      swtpm-overlay
      mailutils-overlay
      notmuch-overlay
#      nixgl-overlay
#      llvm-overlay
      xo-overlay
    ];
  };
in

let
  xo_topdir = toString ./.;

  fonts = pkgs.makeFontsConf {
    fontDirectories = [
      pkgs.inconsolata-lgc
      pkgs.noto-fonts
      pkgs.noto-fonts-emoji
      pkgs.dejavu_fonts
    ];
  };

  # xo deps
  xodeps = [
    pkgs.python3Packages.python
    # note: pybind11 won't build on roly-chicago-24a in nix sandbox, runs out of pty devices
    #
    pkgs.python3Packages.pybind11
    pkgs.llvmPackages_18.llvm.dev
    pkgs.replxx
    pkgs.libwebsockets
    pkgs.jsoncpp
    pkgs.eigen
    pkgs.zlib
    pkgs.libbsd
  ];

  # xo ide utils
  ideutils = [
    pkgs.gsettings-desktop-schemas
    pkgs.emacs30
#    (pkgs.emacs.pkgs.withPackages (epkgs: [
#    ]))
    # pkgs.emacs-pgtk  # wayland with treesitter; alternatively pkgs.emacs30 for emacs+tree-sitter
    pkgs.tree-sitter # CLI tool, but not grammars
    pkgs.notmuch
    pkgs.emacsPackages.notmuch
    pkgs.inconsolata-lgc
    pkgs.fontconfig
  ];

  # xo general-purpose devutils
  devutils = [
    pkgs.nix-tree  # note: needs GHC

    pkgs.git
    pkgs.gh    # github cli
    pkgs.cloc

    pkgs.lcov
    pkgs.catch2
    pkgs.gdb
  ]
  ++ (if pkgs.stdenv.isLinux then [
    pkgs.strace
  ] else [])
  ++ [
    pkgs.which
    pkgs.man
    pkgs.man-pages
    pkgs.less
    pkgs.ripgrep
    pkgs.openssh

    pkgs.ccache
    pkgs.distcc
    pkgs.cmake
    pkgs.pkg-config
    pkgs.unzip
  ];

  # xo docutils
  docutils =
    pkgs.lib.optionals pkgs.stdenv.isLinux [
      pkgs.python3Packages.sphinxcontrib-ditaa
    ] ++
    [
      pkgs.python3Packages.python
      pkgs.python3Packages.pillow
      pkgs.python3Packages.breathe
      #pkgs.python3Packages.sphinx-autobuild   # needs patch for typeguard; defer for now
      pkgs.python3Packages.sphinx-rtd-theme
      pkgs.python3Packages.sphinxcontrib-plantuml
      pkgs.doxygen
      pkgs.graphviz
      pkgs.ditaa
      pkgs.sphinx
    ];

  # xo x11utils
  x11utils =
    [
      pkgs.xorg.xcbutil
      pkgs.xorg.xcbutilwm     # xcb window manager utilities
      pkgs.xorg.xcbutilimage
      pkgs.xorg.xclock
      pkgs.xorg.xdpyinfo
    ];

  # xo opengl utils
  gldeps =
    [
      # glew: used for opengl apps;  not needed for vulkan apps (even when those apps use opengl)
      pkgs.glew
    ] ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
      pkgs.mesa-demos
    ] ++ [
      pkgs.libGL
    ];

  vkdeps =
    pkgs.lib.optionals pkgs.stdenv.isDarwin [
      pkgs.darwin.moltenvk
    ] ++
    [
      pkgs.vulkan-tools
      pkgs.vulkan-loader
      pkgs.vulkan-headers
      pkgs.vulkan-validation-layers
    ];

  imguideps =
    [
#      pkgs.imgui     # incorporated directly into xo dev tree, see xo-imgui/include/imgui
      pkgs.SDL2.dev
      pkgs.xorg.libX11.dev  # e.g. for X11/Xlib.h; needed by SDL2

      pkgs.dejavu_fonts     # also must set DEJAVU_FONTS_PATH for imgui_ex2
    ];

  shell4-assembly =
    { ld-library-path-before,
      vk-icd-filenames,
      shell-hook-text-after,
    } :

    pkgs.mkShell {
      buildInputs = docutils ++ xodeps ++ devutils ++ ideutils ++ x11utils ++ gldeps ++ vkdeps ++ imguideps;

      shellHook =
        let
          # dependencies of host system libraries
          # (e.g. from /usr/lib/x86_64-linux-gnu) that we want to satisfy from nix;
          # sufficient for glxgears
          #
          # be careful here: easy to insert something that breaks xo cmake build
          #
          # with minimal list
          #   (libXau, libXdmcp, libX11, libXext, libXfixes, libXxf86vm, libxml2, libffi,
          #    elfutils, ncurses, expat, zstd, zlib, libbsd, gcc.cc.lib)
          #   glxgears runs at ~170fps
          #
          glpath = pkgs.lib.makeLibraryPath [
            pkgs.wayland         # for libwayland-client.so

            pkgs.xorg.libXau
            pkgs.xorg.libXdmcp
            pkgs.xorg.libX11     # e.g. for libX11-xcb.so
            pkgs.xorg.libXext
            pkgs.xorg.libXfixes
            pkgs.xorg.libXxf86vm
            pkgs.xorg.libxcb

            pkgs.libxml2
            pkgs.libffi

            pkgs.elfutils        # for libelf.so
            pkgs.ncurses         # for libtinfo.so
            pkgs.expat
            pkgs.zstd
            pkgs.zlib            # for libz.so
            pkgs.libbsd

            pkgs.gcc.cc.lib      # for libstdc++.so  (won't blow up cmake, only touching LD_LIBRARY_PATH)
          ];
        in
          ''
        # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
        export CXENV=$CXENV:xo4

        # override SOURCE_DATE_EPOCH to current time (otherwise will get 1980)
        export SOURCE_DATE_EPOCH=$(date +%s)

        # fonts
        export FONTCONFIG_FILE=${fonts}
        export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
        export DEJAVU_FONTS_PATH=${pkgs.dejavu_fonts}/share/fonts
        ${pkgs.fontconfig}/bin/fc-cache -f

        # nix-provided GPU libraries only work out-of-the-box on nixos.
        # For non-nixos build, need to use host-provided versions of these libraries.
        # Complications:
        # 1. host location is likely something like /usr/lib/x86_64-linux-gnu,
        #    in which case interposing that directly will change link behavior for unrelated shared libraries
        #    (e.g. risk getting stale non-nix libc.so etc).  Use a curated symlink directory to finesse.
        # 2. host-installed libraries may not set RUNPATH (they don't need to if installed in system-wide default location).
        #    We need to also add nix-store LD_LIBRARY_PATH entries for indirect dependencies of system-provided libraries.
        #
        export LD_LIBRARY_PATH=${ld-library-path-before}:${glpath}:$LD_LIBRARY_PATH

        export VK_ICD_FILENAMES="${vk-icd-filenames}"

        # need this on OSX, + claude wants it for both wsl2, ubuntu
        export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d

        # hardware acceleration
        export LIBGL_ALWAYS_SOFTWARE=0
        export MESA_LOADER_DRIVER_OVERRIDE=""

        echo "tree_sitter=${pkgs.tree-sitter}"
        tree_sitter=${pkgs.tree-sitter}
        echo "nix_mesa=${pkgs.mesa}"
        nix_mesa=${pkgs.mesa}

        ${shell-hook-text-after}
        '';
    };
in
{
  pkgs = pkgs;
  xo = {
    cmake          = pkgs.xo-cmake;
    indentlog      = pkgs.xo-indentlog;
    refcnt         = pkgs.xo-refcnt;
    subsys         = pkgs.xo-subsys;
    randomgen      = pkgs.xo-randomgen;
    ordinaltree    = pkgs.xo-ordinaltree;
    flatstring     = pkgs.xo-flatstring;
    pyutil         = pkgs.xo-pyutil;
    reflectutil    = pkgs.xo-reflectutil;
    reflect        = pkgs.xo-reflect;
    pyreflect      = pkgs.xo-pyreflect;
    ratio          = pkgs.xo-ratio;
    unit           = pkgs.xo-unit;
    pyunit         = pkgs.xo-pyunit;
    callback       = pkgs.xo-callback;
    webutil        = pkgs.xo-webutil;
    pywebutil      = pkgs.xo-pywebutil;
    printjson      = pkgs.xo-printjson;
    pyprintjson    = pkgs.xo-pyprintjson;
    reactor        = pkgs.xo-reactor;
    pyreactor      = pkgs.xo-pyreactor;
    websock        = pkgs.xo-websock;
    pywebsock      = pkgs.xo-pywebsock;
    statistics     = pkgs.xo-statistics;
    distribution   = pkgs.xo-distribution;
    pydistribution = pkgs.xo-pydistribution;
    simulator      = pkgs.xo-simulator;
    pysimulator    = pkgs.xo-pysimulator;
    process        = pkgs.xo-process;
    pyprocess      = pkgs.xo-pyprocess;
    kalmanfilter   = pkgs.xo-kalmanfilter;
    pykalmanfilter = pkgs.xo-pykalmanfilter;
    expression     = pkgs.xo-expression;
    pyexpression   = pkgs.xo-pyexpression;
    tokenizer      = pkgs.xo-tokenizer;
    reader         = pkgs.xo-reader;
    jit            = pkgs.xo-jit;
    pyjit          = pkgs.xo-pyjit;
  };

  # works, but only if build with --option sandbox=false
  # to workaround pcre broken. overlay above does work,
  # but is not applied to stdenv devs
  #
  shellpre = pkgs.mkShell {
    buildInputs = [
      # NOTE: pkgs.pcre2, pkgs.dejagnu, pkgs.libffi
      # also won't build in genuine sandbox

      pkgs.nix-tree   # needs GHC btw
      pkgs.git
      pkgs.openssh
      pkgs.cloc
      pkgs.lcov
      pkgs.gdb
      pkgs.strace

      pkgs.ripgrep

      pkgs.clang    # 19.1.7
      pkgs.ninja    # will this work? tbd
      pkgs.cmake
      pkgs.distcc
      pkgs.pkg-config
      pkgs.unzip
    ];

    # includes stdenv, so will have gcc,gawk,tar,gzip etc.
    shellHook = ''
      # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
      export CXENV=$CXENV:xopre

      # override SOURCE_DATE_EPOCH to current time (otherwise will get 1980)
      export SOURCE_DATE_EPOCH=$(date +%s)
      '';
  };

  # minimal shell - just enough to verify gcc works.
  # not sufficient to build xo
  #
  # $ nix-shell -A shell0  # requires nixpkgs.runCommand
  #
  shell0 = pkgs.mkShell {
    buildInputs = devutils;

    # includes stdenv, so will have gcc,gawk,tar,gzip etc
    shellHook = ''
      # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
      export CXENV=$CXENV:xo0

      # override SOURCE_DATE_EPOCH to current time (otherwise will get 1980)
      export SOURCE_DATE_EPOCH=$(date +%s)
      '';
  };

  # stable shell.  can build xo without docs
  #
  shell1a = pkgs.mkShell {
    buildInputs = xodeps ++ devutils;

    shellHook = ''
      # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
      export CXENV=$CXENV:xo1a

      # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
      export SOURCE_DATE_EPOCH=$(date +%s)
      '';
  };

  # minimal shell, intended to be stable
  shell1 = pkgs.mkShell {
    buildInputs = docutils ++ xodeps ++ devutils;

    # includes stdenv, so will have gcc,gawk,tar,gzip etc
    shellHook = ''
      # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
      export CXENV=$CXENV:xo1

      # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
      export SOURCE_DATE_EPOCH=$(date +%s)
      '';
  };

  # minimal shell + ide stuff, intended to be stable
  shell2 = pkgs.mkShell {
    buildInputs = docutils ++ xodeps ++ devutils ++ ideutils;

    # includes stdenv, so will have gcc,gawk,tar,gzip etc
    shellHook = ''
      # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
      export CXENV=$CXENV:xo2

      # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
      export SOURCE_DATE_EPOCH=$(date +%s)

      export FONTCONFIG_FILE=${fonts}
      export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
      ${pkgs.fontconfig}/bin/fc-cache -f
      '';
  };

  # xo shell with opengl support
  shell3 = pkgs.mkShell {
    buildInputs = docutils ++ xodeps ++ devutils ++ ideutils ++ x11utils ++ gldeps ++ vkdeps ++ imguideps;

    shellHook =
      let
        # dependencies of host system libraries
        # (e.g. from /usr/lib/x86_64-linux-gnu) that we want to satisfy from nix;
        # sufficient for glxgears
        #
        # be careful here: easy to insert something that breaks xo cmake build
        #
        # with minimal list
        #   (libXau, libXdmcp, libX11, libXext, libXfixes, libXxf86vm, libxml2, libffi,
        #    elfutils, ncurses, expat, zstd, zlib, libbsd, gcc.cc.lib)
        #   glxgears runs at ~170fps
        #
        glpath = pkgs.lib.makeLibraryPath [
          pkgs.wayland         # for libwayland-client.so

          pkgs.xorg.libXau
          pkgs.xorg.libXdmcp
          pkgs.xorg.libX11     # e.g. for libX11-xcb.so
          pkgs.xorg.libXext
          pkgs.xorg.libXfixes
          pkgs.xorg.libXxf86vm

          pkgs.libxml2
          pkgs.libffi

          pkgs.elfutils        # for libelf.so
          pkgs.ncurses         # for libtinfo.so
          pkgs.expat
          pkgs.zstd
          pkgs.zlib            # for libz.so
          pkgs.libbsd

          pkgs.gcc.cc.lib      # for libstdc++.so  (won't blow up cmake, only touching LD_LIBRARY_PATH)
        ];
      in
        ''
        # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
        export CXENV=$CXENV:xo3

        # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
        export SOURCE_DATE_EPOCH=$(date +%s)

        # fonts
        export FONTCONFIG_FILE=${fonts}
        export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
        ${pkgs.fontconfig}/bin/fc-cache -f

        # nix-provided GPU libraries only work out-of-the-box on nixos.
        # For non-nixos build, need to use host-provided versions of these libraries.
        # Complications:
        # 1. host location is likely something like /usr/lib/x86_64-linux-gnu,
        #    in which case interposing that directly will change link behavior for unrelated shared libraries
        #    (e.g. risk getting stale non-nix libc.so etc).  Use a curated symlink directory to finesse.
        # 2. host-installed libraries may not set RUNPATH (they don't need to if installed in system-wide default location).
        #    We need to also add nix-store LD_LIBRARY_PATH entries for indirect dependencies of system-provided libraries.
        #
        export LD_LIBRARY_PATH=/usr/lib/wsl/lib:${xo_topdir}/etc/hostwsl:${glpath}:$LD_LIBRARY_PATH

        # for vulkan on wsl, need to use llvm pipe rendering.
        # (asof sep 2025 hardware rendering is supported, but with compute-only in mind i.e. for cuda)
        # export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json
        export VK_ICD_FILENAMES="${pkgs.mesa}/share/vulkan/icd.d/dzn_icd.x86_64.json"

        # need this on OSX, + claude wants it for wsl2.  but looks sketchy to me
        export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d

        # hardware acceleration
        export LIBGL_ALWAYS_SOFTWARE=0
        export MESA_LOADER_DRIVER_OVERRIDE=""

        # wsl2-specific gpu setup
        export MESA_D3D12_DEFAULT_ADAPTER_NAME=DX

        echo "using d3d12 vulkan driver: $VK_ICD_FILENAMES"
        '';

    # TODO: consider a nix project to generate this directory. nixwsl
  };

  # vkcube works here
  shell4-wsl = shell4-assembly {
    ld-library-path-before = "${pkgs.mesa}/lib:/usr/lib/wsl/lib:${xo_topdir}/etc/hostwsl2";
    vk-icd-filenames = "${pkgs.mesa}/share/vulkan/icd.d/dzn_icd.x86_64.json";
    shell-hook-text-after = ''
      export MESA_D3D12_DEFAULT_ADAPTER_NAME=DX
      echo "using d3d12 vulkan driver: $VK_ICD_FILENAMES"
    '';
  };

  # nov 2025 - try to get xo-imgui working on native ubuntu
  shell4-nvidia = shell4-assembly {
    ld-library-path-before = "${xo_topdir}/etc/hostubuntu:${pkgs.mesa}/lib";
    vk-icd-filenames = "/usr/share/vulkan/icd.d/nvidia_icd.json";
    shell-hook-text-after = ''
      export __GLX_VENDOR_LIBRARY_NAME=nvidia
      #export __EGL_VENDOR_LIBRARY_DIRS=/usr/share/glvnd/egl_vendor.d  # maybe
      echo "using nvidia for libglvnd"
    '';
  };

  # nov 2025 - try to get xo-imgui working on osx again
  # WIP !  plan:
  #  1 - get this to work as standalone copy of shell4-assembly
  #  2 - merge to look like shell4-wsl / shell4-nvidia
  #
  shell4-osx = pkgs.mkShell {
    buildInputs = docutils ++ xodeps ++ devutils ++ ideutils ++ x11utils ++ gldeps ++ vkdeps ++ imguideps;

    shellHook =
      ''
      export CXENV=$CXENV:xo4-WIP

        # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
        export SOURCE_DATE_EPOCH=$(date +%s)

        # fonts
        export FONTCONFIG_FILE=${fonts}
        export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
        export DEJAVU_FONTS_PATH=${pkgs.dejavu_fonts}/share/fonts
        ${pkgs.fontconfig}/bin/fc-cache -f

      # would set vk-icd-filename to this + call shell4-assembly
      export VK_ICD_FILENAMES=${pkgs.darwin.moltenvk}/share/vulkan/icd.d/MoltenVK_icd.json

      # already baked in to shell4-assembly
      export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d

      # dependencies on host system libraries
      # glpath = pkgs.lib.makeLibraryPath [ ];
      '';
  };

  # like shell4 but drop etc/hostwsl2 symlink dir.
  # looks like nixpkgs mesa not built for wsl2 dxg
  # works with opengl (llvmpipe) and vulkan
  #
  shell5 = pkgs.mkShell {
    buildInputs = docutils ++ xodeps ++ devutils ++ ideutils ++ x11utils ++ gldeps ++ vkdeps ++ imguideps ++ imguideps;

    shellHook =
      let
        # dependencies of host system libraries
        # (e.g. from /usr/lib/x86_64-linux-gnu) that we want to satisfy from nix;
        # sufficient for glxgears
        #
        # be careful here: easy to insert something that breaks xo cmake build
        #
        # with minimal list
        #   (libXau, libXdmcp, libX11, libXext, libXfixes, libXxf86vm, libxml2, libffi,
        #    elfutils, ncurses, expat, zstd, zlib, libbsd, gcc.cc.lib)
        #   glxgears runs at ~170fps
        #
        glpath = pkgs.lib.makeLibraryPath [
          pkgs.wayland         # for libwayland-client.so

          pkgs.mesa

          pkgs.xorg.libXau
          pkgs.xorg.libXdmcp
          pkgs.xorg.libX11     # e.g. for libX11-xcb.so
          pkgs.xorg.libXext
          pkgs.xorg.libXfixes
          pkgs.xorg.libXxf86vm
          pkgs.xorg.libxcb

          pkgs.libxml2
          pkgs.libffi

          pkgs.elfutils        # for libelf.so
          pkgs.ncurses         # for libtinfo.so
          pkgs.expat
          pkgs.zstd
          pkgs.zlib            # for libz.so
          pkgs.libbsd

          pkgs.gcc.cc.lib      # for libstdc++.so  (won't blow up cmake, only touching LD_LIBRARY_PATH)
        ];
      in
        ''
        # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
        export CXENV=$CXENV:xo5

        # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
        export SOURCE_DATE_EPOCH=$(date +%s)

        # fonts
        export FONTCONFIG_FILE=${fonts}
        export FONTCONFIG_PATH=${pkgs.fontconfig.out}/etc/fonts
        export DEJAVU_FONTS_PATH=${pkgs.dejavu_fonts}/share/fonts
        ${pkgs.fontconfig}/bin/fc-cache -f

        # works but only get glx using llvmpipe.
        # vkcube uses dzn (see VK_ICD_FILENAMES) however!
        export LD_LIBRARY_PATH=/usr/lib/wsl/lib:${glpath}:$LD_LIBRARY_PATH

        # need this on OSX, + claude wants it for wsl2.  but looks sketchy to me
        export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
        export VK_ICD_FILENAMES="${pkgs.mesa}/share/vulkan/icd.d/dzn_icd.x86_64.json"

        # hardware acceleration (so far ineffective w/ nixpkgs mesa)
        export LIBGL_ALWAYS_SOFTWARE=0
        export MESA_LOADER_DRIVER_OVERRIDE="d3d12"

        # wsl2-specific gpu setup (so far ineffective w/ nixpkgs mesa)
        export MESA_D3D12_DEFAULT_ADAPTER_NAME=DX

        echo "using d3d12 vulkan driver: $VK_ICD_FILENAMES"

        echo "nix_mesa=${pkgs.mesa}"
        nix_mesa=${pkgs.mesa}

        echo "nix_libgl=${pkgs.libGL}"
        nix_libgl=${pkgs.libGL}

        echo "nix_vk_validation: VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d"
        nix_vk_validation=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
        '';

        # don't seem to need this -- doesn't invoke mesa hardware accel
        #echo "nix_nixgl=${pkgs.nixgl.nixGLMesa}"
        #nix_nixgl=${pkgs.nixgl.nixGLMesa}
  };

  # Old shell from failed vulkan-on-wsl attempts.
  # Descends from working OSX though, so keep until we coordinate with that
  #
  shell = pkgs.mkShell {
    # maybe should create a python environment:
    #   let
    #     pythonenv = pkgs.python3.withPackages (pset: [ pset.sphinx pset.breathe ..etc.. ])
    #   in
    #     buildInputs = [ pythonenv ..etc.. ];
    #

    buildInputs = [
      #pkgs.nix

      pkgs.nushell
    ]
    ++ docutils
    ++ devutils
    ++ xodeps
    ++ ideutils
    ++ (if pkgs.stdenv.isLinux then [ pkgs.emscripten ] else [])
    ++ x11utils
    ++ gldeps
    ++ imguideps
    ++
    [
      #     pkgs.nixgl.auto.nixGLDefault  # this + libGL + mesa-demos -> 'nixGL glxgears' works out-of-the-box
      #    pkgs.nixgl.auto.nixGLNvidia
      #    pkgs.nixgl.nixVulkanIntel  # mesa+vulkan
      #     pkgs.nixgl.nixGLMesa
      #    pkgs.nixgl.nixGLNvidia   # try this explicitly for wsl2
      # *might* also need xorg.libXext.dev, xorg.libXrender.dev, xorg.xorgproto
    ]
    ++ (if pkgs.stdenv.isLinux then [ pkgs.stdenv.cc.cc.lib pkgs.glibc ] else [])
    ++ vkdeps
    ++ (if pkgs.stdenv.isDarwin then [ pkgs.darwin.moltenvk ] else [])
    ++ [
      pkgs.xorg.xcbutil
      pkgs.xorg.xcbutilwm     # xcb window manager utilities
      pkgs.xorg.xcbutilimage

      # fonts for imgui
      #    pkgs.gucharmap
      pkgs.noto-fonts
      #pkgs.noto-fonts-lgc   # lgc for latin,greek,cyrillic   (but doesn't exist in pinned nixpkgs)
      #pkgs.noto-fonts-cjk   # cjk for chinese,japanese,korean
      pkgs.noto-fonts-emoji
      pkgs.dejavu_fonts
    ];

    shellHook = ''
    # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
    export CXENV=$CXENV:xo

    # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
    export SOURCE_DATE_EPOCH=$(date +%s)

    # software-only pipeline
    #export SDL_VIDEODRIVER=x11
    #export LIBGL_ALWAYS_SOFTWARE=1

    #export SDL_VIDEO_X11_FORCE_EGL=0
    #export SDL_VIDEO_X11_VISUALID=0x023

    #export LIBGL_ALWAYS_INDIRECT=1
    #export WSLG_FORCE_EGL=1
    #export GLFW_USE_EGL=1

    export VULKAN_SDK=${pkgs.vulkan-headers}

    # for Vulkan on OSX only..
    ${pkgs.lib.optionalString pkgs.stdenv.isDarwin ''
    export VK_ICD_FILENAMES=${pkgs.darwin.moltenvk}/share/vulkan/icd.d/MoltenVK_icd.json
    export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
    ''}

    # do need VK_LAYER_PATH on OSX.  Not sure if we need it on wsl2
    #export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d

    # nixGL experiment (on wsl2).  Doesn't work out of the box, but maybe with some help..
    # TODO: put this in its own generated wrapper.
    # TODO: if/when this works, refactor to use something like
    #    let vklibpath = pkgs.lib.makeLibraryPath [ pkgs.foo pkgs.libffi pkgs.glibc ... ];
    #  with
    #    export LD_LIBRARY_PATH="$vklibpath:$LD_LIBRARY_PATH"
    # NOTE: nixGL doesn't really know how to handle a wsl2 host, in particular doesn't know
    #       about /usr/lib/wsl/lib
    #
    ${pkgs.lib.optionalString pkgs.stdenv.isLinux ''
#    export LD_LIBRARY_PATH=${pkgs.glibc}:$LD_LIBRARY_PATH
#    export LD_LIBRARY_PATH=${xo_topdir}/etc/hostwsl:$LD_LIBRARY_PATH
#    export LD_LIBRARY_PATH=${pkgs.vulkan-loader}/lib:$LD_LIBRARY_PATH
#    export LD_LIBRARY_PATH=/usr/lib/wsl/lib:$LD_LIBRARY_PATH
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.mesa}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libffi}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.wayland}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXext}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXau}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXdmcp}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXfixes}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXxf86vm}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libX11}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.gcc.cc.lib}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.ncurses}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.expat}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.elfutils.out}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libxml2.out}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.icu}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libmd}
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xz.out}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.zstd.out}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.zlib}/lib
#    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libbsd}/lib
#    #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libedit}/lib   # gets libedit.so.0, but system wants libedit.so.2
#    #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.lm_sensors}    # wanted libsensors.so.5, this package doesn't provide it

    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

    # lvp_icd.x86_64.json: llvm pipe rendering (software)
    export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json

    export VK_LD_PRELOAD=${pkgs.vulkan-loader}/lib/libvulkan.so.1

    ## mesa software rendering settings
    # export GALLIUM_DRIVER=llvmpipe
    # export MESA_GL_VERSION_OVERRIDE=3.3
    # export LIBGL_ALWAYS_SOFTWARE=1

    # export VK_LOADER_DEBUG=all   # or info
    # export MESA_DEBUG=1
    # export EGL_LOG_LEVEL=debug
    # export LIBGL_DEBUG=verbose
    # vkcube --wsi xcb
    ''}

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
  };
}
