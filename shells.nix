# shells.nix - dev shell definitions
# Accepts a pkgs instance (xo-overlay applied internally)
# Returns attrset of dev shells + xo package references
#
{ pkgs ? import <nixpkgs> {} }:

let
  # rename, we're about to shadow
  pkgs0 = pkgs;
in
let
  # apply xo package overlay; makes pkgs.xo-* available
  pkgs = pkgs0.extend (self: super: import ./xo.nix { pkgs = self; });
  # bona fide bug in git 2.50.1
  # bug when using:
  #   git subtree split ...
  # git subtree needs to review a set of commits, filtering for those that
  # intersect with target prefix. e.g.
  #   git subtree split --prefix=xo-indentlog -b _demux/xo-indentlog
  #
  # 1. the split function is building a map old->new from commits in current repo,
  #    to a smaller set of commits in the split repo:
  #    it intends finding commits that intersect with anything in prefix,
  #    adding old->new map entries for each such commit.
  #
  # 2. subtree (incorrectly?) decides it needs to cache a commit that does not
  #    actually intersect prefix.  Calls its cache_set() twice with the same newrev.
  #    2nd calls aborts, assumes cache should not already contain an oldrev key.
  #
  # 3. fix is to permit multiple calls to cache_set() with the same old->new
  #    mapping.  Keep the abort if called twice with different target commits.
  #
  patched-git = pkgs.git.overrideAttrs (old: {
      postInstall = (old.postInstall or "") + ''
        patch $out/libexec/git-core/git-subtree ${./patches/git-subtree-cache-fix.patch}
      '';
    });

  xo_topdir = toString ./.;

  fonts = pkgs.makeFontsConf {
    fontDirectories = [
      pkgs.inconsolata-lgc
      pkgs.noto-fonts
      pkgs.noto-fonts-color-emoji
      pkgs.dejavu_fonts
    ];
  };

  # xo deps.
  # (we don't need this for nix-build.
  #  load-bearing for nix development shell driving a cmake build)
  #
  xodeps = [
    pkgs.python3Packages.python
    # note: pybind11 won't build on roly-chicago-24a in nix sandbox, runs out of pty devices
    #
    pkgs.python3Packages.pybind11
    pkgs.python3Packages.json5
    pkgs.python3Packages.jinja2
    pkgs.llvmPackages_18.llvm.dev
    pkgs.replxx
    pkgs.libwebsockets
    pkgs.jsoncpp
    pkgs.eigen
    pkgs.libunwind
  ] ++ (if pkgs.stdenv.isLinux then [
    pkgs.elfutils.dev    # for libdw (DWARF debug info lookup)
  ] else [
    pkgs.llvmPackages_18.libunwind.dev
  ]) ++ [
    pkgs.zlib
    pkgs.libbsd
  ];

  # emacs
  emacs4xo = (pkgs.emacsPackagesFor pkgs.emacs30).emacsWithPackages (epkgs: with epkgs; [
    lsp-mode
    lsp-ui
    lsp-ivy
    flycheck
    ivy
    rg
    transient   # magit dep
    nix-mode
    yaml-mode
    htmlize
    magit
    exec-path-from-shell
    highlight
    surround
    projectile
    treemacs
    treemacs-projectile
    company
    cmake-mode
    which-key
    xterm-color
    yasnippet
  ] ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
    notmuch
  ]);

  # xo ide utils
  ideutils = [
    pkgs.gsettings-desktop-schemas
    emacs4xo
    # pkgs.emacs-pgtk  # wayland with treesitter; alternatively pkgs.emacs30 for emacs+tree-sitter
    pkgs.tree-sitter # CLI tool, but not grammars
    pkgs.llvmPackages_18.clang-tools  # clangd for lsp-mode
    pkgs.notmuch
    pkgs.emacsPackages.notmuch
    pkgs.inconsolata-lgc
    pkgs.fontconfig
  ]
  ++ (if pkgs.stdenv.isLinux then [
    pkgs.ghostty
  ] else [])
  ++ [
    pkgs.timg
    pkgs.fish
    pkgs.nushell
  ];

  # xo general-purpose devutils
  devutils = [
    pkgs.nix-tree  # note: needs GHC
    #pkgs.nix

    patched-git # instead of pkgs.git
    pkgs.gh    # github cli
    pkgs.cloc
    pkgs.bloaty

    pkgs.catch2
    pkgs.cli11

    pkgs.btop
  ]
  ++ (if pkgs.stdenv.isLinux then [
    pkgs.gdb
    pkgs.strace
    pkgs.lcov                    # gcov-format coverage post-processing
  ] else [
    pkgs.llvmPackages_18.lldb
    pkgs.llvmPackages_18.llvm    # llvm-profdata, llvm-cov for source-based coverage
  ])
  ++ [
    pkgs.which
    pkgs.man
    pkgs.man-pages
    pkgs.less
    pkgs.ripgrep
    pkgs.openssh

    pkgs.ccache
  ]
  ++ (if pkgs.stdenv.isLinux then [
    pkgs.distcc
  ] else [])
  ++ [
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
    [ pkgs.vulkan-tools
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
          glpath = pkgs.lib.makeLibraryPath ([
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

          ] ++ (if pkgs.stdenv.isLinux then [ pkgs.elfutils ] else [])
          ++ [
            pkgs.ncurses         # for libtinfo.so
            pkgs.expat
            pkgs.zstd
            pkgs.zlib            # for libz.so
            pkgs.libbsd

            pkgs.gcc.cc.lib      # for libstdc++.so  (won't blow up cmake, only touching LD_LIBRARY_PATH)
          ]);
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
  inherit pkgs;

  # load-bearing for
  #   $ nix-build -A xo.foo
  #
  xo = {
    cmake          = pkgs.xo-cmake;
    subsys         = pkgs.xo-subsys;
    indentlog      = pkgs.xo-indentlog;
    randomgen      = pkgs.xo-randomgen;
    flatstring     = pkgs.xo-flatstring;
    reflectutil    = pkgs.xo-reflectutil;
    arena          = pkgs.xo-arena;
    facet          = pkgs.xo-facet;
    printable2     = pkgs.xo-printable2;
    testutil       = pkgs.xo-testutil;
    alloc2         = pkgs.xo-alloc2;
    refcnt         = pkgs.xo-refcnt;
    pyutil         = pkgs.xo-pyutil;
    reflect        = pkgs.xo-reflect;
    pyreflect      = pkgs.xo-pyreflect;
    stringtable2   = pkgs.xo-stringtable2;
    object2        = pkgs.xo-object2;
    gc             = pkgs.xo-gc;
    type           = pkgs.xo-type;
    procedure2     = pkgs.xo-procedure2;
    numeric        = pkgs.xo-numeric;
    tokenizer2     = pkgs.xo-tokenizer2;
    expression2    = pkgs.xo-expression2;
    reader2        = pkgs.xo-reader2;
    interpreter2   = pkgs.xo-interpreter2;

    allocutil      = pkgs.xo-allocutil;
    alloc          = pkgs.xo-alloc;
    ratio          = pkgs.xo-ratio;
    unit           = pkgs.xo-unit;
    pyunit         = pkgs.xo-pyunit;
    callback       = pkgs.xo-callback;
    object         = pkgs.xo-object;
    ordinaltree    = pkgs.xo-ordinaltree;
    expression     = pkgs.xo-expression;
    pyexpression   = pkgs.xo-pyexpression;
    tokenizer      = pkgs.xo-tokenizer;
    reader         = pkgs.xo-reader;
    jit            = pkgs.xo-jit;
    pyjit          = pkgs.xo-pyjit;

    webutil        = pkgs.xo-webutil;
    pywebutil      = pkgs.xo-pywebutil;
    printjson      = pkgs.xo-printjson;
    pyprintjson    = pkgs.xo-pyprintjson;
    reactor        = pkgs.xo-reactor;
    pyreactor      = pkgs.xo-pyreactor;
    websock        = pkgs.xo-websock;
    pywebsock      = pkgs.xo-pywebsock;
    process        = pkgs.xo-process;
    pyprocess      = pkgs.xo-pyprocess;
    statistics     = pkgs.xo-statistics;
    distribution   = pkgs.xo-distribution;
    pydistribution = pkgs.xo-pydistribution;
    simulator      = pkgs.xo-simulator;
    pysimulator    = pkgs.xo-pysimulator;
    kalmanfilter   = pkgs.xo-kalmanfilter;
    pykalmanfilter = pkgs.xo-pykalmanfilter;
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
      patched-git  # instead of pkgs.git
      pkgs.git-subrepo
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
        glpath = pkgs.lib.makeLibraryPath ([
          pkgs.wayland         # for libwayland-client.so

          pkgs.xorg.libXau
          pkgs.xorg.libXdmcp
          pkgs.xorg.libX11     # e.g. for libX11-xcb.so
          pkgs.xorg.libXext
          pkgs.xorg.libXfixes
          pkgs.xorg.libXxf86vm

          pkgs.libxml2
          pkgs.libffi

        ] ++ (if pkgs.stdenv.isLinux then [ pkgs.elfutils ] else [])
        ++ [
          pkgs.ncurses         # for libtinfo.so
          pkgs.expat
          pkgs.zstd
          pkgs.zlib            # for libz.so
          pkgs.libbsd

          pkgs.gcc.cc.lib      # for libstdc++.so  (won't blow up cmake, only touching LD_LIBRARY_PATH)
        ]);
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
  #shell4-osx = pkgs.mkShell.override { stdenv = pkgs.jitStdenv; } { .. } # very high attack surface area
  shell4-osx =
    let
      clangStdenv = pkgs.overrideCC pkgs.stdenv pkgs.llvmPackages_18.clang;
    in
      let jitStdenv = if pkgs.stdenv.isDarwin then clangStdenv else pkgs.stdenv;
    in
      (pkgs.mkShell.override { stdenv = jitStdenv; })
        {
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

      # debugserver interacts with a local process to debug it.
      # requires code signing, so nix can't replicate it.
      export LLDB_DEBUGSERVER_PATH=/Library/Developer/CommandLineTools/Library/PrivateFrameworks/LLDB.framework/Versions/A/Resources/debugserver

      # choose a temp directory that can outlive nix-shell
      export TMPDIR=$(getconf DARWIN_USER_TEMP_DIR)
      export TMP=$TMPDIR
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
    ]
    ++ (if pkgs.stdenv.isLinux then [ pkgs.stdenv.cc.cc.lib pkgs.glibc ] else [])
    ++ vkdeps
    ++ (if pkgs.stdenv.isDarwin then [ pkgs.darwin.moltenvk ] else [])
    ++ [
      pkgs.xorg.xcbutil
      pkgs.xorg.xcbutilwm     # xcb window manager utilities
      pkgs.xorg.xcbutilimage

      # fonts for imgui
      pkgs.noto-fonts
      pkgs.noto-fonts-color-emoji
      pkgs.dejavu_fonts
    ];

    shellHook = ''
    # CXENV: cosmetic: coordinates with ~/proj/env/dotfiles/bashrc to drive PS1
    export CXENV=$CXENV:xo

    # override SOUCE_DATE_EPOCH to current time (otherwise will get 1980)
    export SOURCE_DATE_EPOCH=$(date +%s)

    export VULKAN_SDK=${pkgs.vulkan-headers}

    # for Vulkan on OSX only..
    ${pkgs.lib.optionalString pkgs.stdenv.isDarwin ''
    export VK_ICD_FILENAMES=${pkgs.darwin.moltenvk}/share/vulkan/icd.d/MoltenVK_icd.json
    export VK_LAYER_PATH=${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d
    ''}

    ${pkgs.lib.optionalString pkgs.stdenv.isLinux ''
    export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

    # lvp_icd.x86_64.json: llvm pipe rendering (software)
    export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json

    export VK_LD_PRELOAD=${pkgs.vulkan-loader}/lib/libvulkan.so.1
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
