{
  description = "xo-unit: c++ compile-time dimension checking and unit conversion";

  # Adapted from xo-nix2/flake.nix

  # MANIFESTO
  # No build instructions in flake.nix
  # - Following Jade Lovelace's advice
  # - Build instructions are in pkgs/*.nix
  # - Each pkgs/*.nix is intended to work 'like a .nix file in nixpkgs'
  #   I'm being lazy about source hashes,  since flake.nix supplies them.
  #
  # Motivation (per JL) versus doing everything in flake.nix:
  # - nixpkgs-ready
  # - parameterized
  # - overridable
  # - still works if cross-compiling
  #
  # Instead:  using flake.nix as entry point:
  # - pin nixpkgs to a specific revision,  for reproducibility
  # - pin our candidate packages (pkgs/*.nix),  for the same reason.

  # to determine specific hash for nixpkgs:
  # 1. $ cd ~/proj/nixpkgs
  # 2. $ git checkout release-23.05
  # 3. $ git fetch
  # 4. $ git pull
  # 5. $ git log -1
  #    take this hash,  then substitue for ${hash} in:
  #      inputs.nixpkgs.url = "https://github.com/NixOS/nixpkgs/archive/${hash}.tar.gz";
  #    below
  #inputs.nixpkgs.url = "https://github.com/NixOS/nixpkgs/archive/9a333eaa80901efe01df07eade2c16d183761fa3.tar.gz";

  # as sbove but instead of {release-23.05} use {release-23.11}
  #   gcc -> 12.3.0
  #   python -> 3.11.6
  #
  inputs.nixpkgs.url = "https://github.com/NixOS/nixpkgs/archive/217b3e910660fbf603b0995a6d2c3992aef4cc37.tar.gz"; # asof 10mar2024
  #inputs.nixpkgs.url = "https://github.com/NixOS/nixpkgs/archive/4dd376f7943c64b522224a548d9cab5627b4d9d6.tar.gz";

  # inputs.nixpkgs.url
  #   = "https://github.com/NixOS/nixpkgs/archive/fac3684647cc9d6dfb2a39f3f4b7cf5fc89c96b6.tar.gz"; # asof 8feb2024
  # fac3684647.. asof 17oct2023
  # instead of
  #   inputs.nixpkgs.url = "github:nixos/nixpkgs/23.05";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  # To add a new package,  visit placeholder-A .. placeholder-E

  inputs.xo-cmake-path          = { type = "github"; owner = "Rconybea"; repo = "xo-cmake";          flake = false; };
  inputs.xo-indentlog-path      = { type = "github"; owner = "Rconybea"; repo = "indentlog";         flake = false; };
  inputs.xo-refcnt-path         = { type = "github"; owner = "Rconybea"; repo = "refcnt";            flake = false; };
  inputs.xo-subsys-path         = { type = "github"; owner = "Rconybea"; repo = "subsys";            flake = false; };
  inputs.xo-randomgen-path      = { type = "github"; owner = "Rconybea"; repo = "randomgen";         flake = false; };
  inputs.xo-ordinaltree-path    = { type = "github"; owner = "Rconybea"; repo = "xo-ordinaltree";    flake = false; };
  inputs.xo-pyutil-path         = { type = "github"; owner = "Rconybea"; repo = "xo-pyutil";         flake = false; };
  inputs.xo-reflect-path        = { type = "github"; owner = "Rconybea"; repo = "reflect";           flake = false; };
  inputs.xo-pyreflect-path      = { type = "github"; owner = "Rconybea"; repo = "xo-pyreflect";      flake = false; };
  inputs.xo-printjson-path      = { type = "github"; owner = "Rconybea"; repo = "xo-printjson";      flake = false; };
  inputs.xo-pyprintjson-path    = { type = "github"; owner = "Rconybea"; repo = "xo-pyprintjson";    flake = false; };
  inputs.xo-callback-path       = { type = "github"; owner = "Rconybea"; repo = "xo-callback";       flake = false; };
  inputs.xo-webutil-path        = { type = "github"; owner = "Rconybea"; repo = "xo-webutil";        flake = false; };
  inputs.xo-pywebutil-path      = { type = "github"; owner = "Rconybea"; repo = "xo-pywebutil";      flake = false; };
  inputs.xo-reactor-path        = { type = "github"; owner = "Rconybea"; repo = "xo-reactor";        flake = false; };
  inputs.xo-pyreactor-path      = { type = "github"; owner = "Rconybea"; repo = "xo-pyreactor";      flake = false; };
  inputs.xo-simulator-path      = { type = "github"; owner = "Rconybea"; repo = "xo-simulator";      flake = false; };
  inputs.xo-pysimulator-path    = { type = "github"; owner = "Rconybea"; repo = "xo-pysimulator";    flake = false; };
  inputs.xo-distribution-path   = { type = "github"; owner = "Rconybea"; repo = "xo-distribution";   flake = false; };
  inputs.xo-pydistribution-path = { type = "github"; owner = "Rconybea"; repo = "xo-pydistribution"; flake = false; };
  inputs.xo-process-path        = { type = "github"; owner = "Rconybea"; repo = "xo-process";        flake = false; };
  inputs.xo-pyprocess-path      = { type = "github"; owner = "Rconybea"; repo = "xo-pyprocess";      flake = false; };
  inputs.xo-statistics-path     = { type = "github"; owner = "Rconybea"; repo = "xo-statistics";     flake = false; };
  inputs.xo-kalmanfilter-path   = { type = "github"; owner = "Rconybea"; repo = "xo-kalmanfilter";   flake = false; };
  inputs.xo-pykalmanfilter-path = { type = "github"; owner = "Rconybea"; repo = "xo-pykalmanfilter"; flake = false; };
  inputs.xo-websock-path        = { type = "github"; owner = "Rconybea"; repo = "xo-websock";        flake = false; };
  inputs.xo-pywebsock-path      = { type = "github"; owner = "Rconybea"; repo = "xo-pywebsock";      flake = false; };
  # placeholder-A

  outputs
  = { self,
      nixpkgs,
      flake-utils,
      xo-cmake-path,
      xo-indentlog-path,
      xo-refcnt-path,
      xo-subsys-path,
      xo-reflect-path,
      xo-pyreflect-path,
      xo-randomgen-path,
      xo-ordinaltree-path,
      xo-pyutil-path,
      xo-printjson-path,
      xo-pyprintjson-path,
      xo-callback-path,
      xo-webutil-path,
      xo-pywebutil-path,
      xo-reactor-path,
      xo-pyreactor-path,
      xo-simulator-path,
      xo-pysimulator-path,
      xo-distribution-path,
      xo-pydistribution-path,
      xo-process-path,
      xo-pyprocess-path,
      xo-statistics-path,
      xo-kalmanfilter-path,
      xo-pykalmanfilter-path,
      xo-websock-path,
      xo-pywebsock-path,
      # placeholder-B
    } :
      # out :: system -> {packages, devShells}
      let
        out
          = system :
            let
              pkgs = nixpkgs.legacyPackages.${system};

              # could try using
              #   appliedOverlay = (pkgs.extend self.overlays.default)
              # but it doesn't seem to work the way I expect,
              # For example, wants to pickup 2.7.11 python for xo-pyutil !
              #
              appliedOverlay = self.overlays.default pkgs pkgs;

            in
              {
                #xo-cmake-dir = "${self.packages.${system}.xo-cmake}/share/cmake";

                # reminder:
                # 'packages' comprises the output of this flake;
                # each defn invokes a build
                #   ./pkgs/$example.nix
                # using
                #   cmake-examples-$example-path
                # above for source code

                packages.xo-cmake = appliedOverlay.xo-cmake;
                packages.xo-indentlog = appliedOverlay.xo-indentlog;
                packages.xo-refcnt = appliedOverlay.xo-refcnt;
                packages.xo-subsys = appliedOverlay.xo-subsys;
                packages.xo-randomgen = appliedOverlay.xo-randomgen;
                packages.xo-ordinaltree = appliedOverlay.xo-ordinaltree;
                packages.xo-pyutil = appliedOverlay.xo-pyutil;
                packages.xo-reflect = appliedOverlay.xo-reflect;
                packages.xo-pyreflect = appliedOverlay.xo-pyreflect;
                packages.xo-printjson = appliedOverlay.xo-printjson;
                packages.xo-pyprintjson = appliedOverlay.xo-pyprintjson;
                packages.xo-callback = appliedOverlay.xo-callback;
                packages.xo-webutil = appliedOverlay.xo-webutil;
                packages.xo-pywebutil = appliedOverlay.xo-pywebutil;
                packages.xo-reactor = appliedOverlay.xo-reactor;
                packages.xo-pyreactor = appliedOverlay.xo-pyreactor;
                packages.xo-simulator = appliedOverlay.xo-simulator;
                packages.xo-pysimulator = appliedOverlay.xo-pysimulator;
                packages.xo-distribution = appliedOverlay.xo-distribution;
                packages.xo-pydistribution = appliedOverlay.xo-pydistribution;
                packages.xo-process = appliedOverlay.xo-process;
                packages.xo-pyprocess = appliedOverlay.xo-pyprocess;
                packages.xo-statistics = appliedOverlay.xo-statistics;
                packages.xo-kalmanfilter = appliedOverlay.xo-kalmanfilter;
                packages.xo-pykalmanfilter = appliedOverlay.xo-pykalmanfilter;
                packages.xo-websock = appliedOverlay.xo-websock;
                packages.xo-pywebsock = appliedOverlay.xo-pywebsock;
                # placeholder-C

                packages.xo-userenv = appliedOverlay.xo-userenv;

                devShells = appliedOverlay.devShells;
              };
      in
        flake-utils.lib.eachDefaultSystem
          out
        //
        {
          # introduce overlay to extend nixpkgs with our local packages,
          # (which ofc are not present in nixpkgs,  though same form would work if they were present)
          #
          overlays.default = final: prev:
            (
              let
                # can use
                #  $ nix-env -qaP | grep \.boost            # show known boost versions
                #  $ nix-env -qaP | grep \.python.*Packages # show known python versions

                stdenv = prev.stdenv;

                boost = prev.boost182;
                python = prev.python311Full;
                pythonPackages = prev.python311Packages;
                #doxygen = prev.doxygen;

                pybind11 = pythonPackages.pybind11;
                #breathe = python3Packages.breathe;
                #sphinx = python3Packages.sphinx;
                #sphinx-rtd-theme = python3Packages.sphinx-rtd-theme;

                #extras1 = { boost = boost; };
                #extras2 = { boost = boost; python3Packages = python3Packages; pybind11 = pybind11; };
                #extras3 = { boost = boost; python3Packages = python3Packages; pybind11 = pybind11; doxygen = doxygen; };
                #extras4 = extras3 // { breathe = breathe; };

                xo-cmake =
                  (prev.callPackage ./pkgs/xo-cmake.nix {}).overrideAttrs
                    (old: { src = xo-cmake-path; });

                xo-indentlog =
                  (prev.callPackage ./pkgs/xo-indentlog.nix { xo-cmake = xo-cmake; }).overrideAttrs
                    (old: { src = xo-indentlog-path; });

                xo-subsys =
                  (prev.callPackage ./pkgs/xo-subsys.nix { xo-cmake = xo-cmake; }).overrideAttrs
                    (old: { src = xo-subsys-path; });

                xo-refcnt =
                  (prev.callPackage ./pkgs/xo-refcnt.nix { xo-cmake = xo-cmake;
                                                           xo-indentlog = xo-indentlog; }).overrideAttrs
                    (old: { src = xo-refcnt-path; });

                xo-randomgen =
                  (prev.callPackage ./pkgs/xo-randomgen.nix { xo-cmake = xo-cmake;
                                                              xo-indentlog = xo-indentlog; }).overrideAttrs
                    (old: { src = xo-randomgen-path; });

                xo-ordinaltree =
                  (prev.callPackage ./pkgs/xo-ordinaltree.nix { xo-cmake = xo-cmake;
                                                                xo-refcnt = xo-refcnt;
                                                                xo-randomgen = xo-randomgen; }).overrideAttrs
                    (old: { src = xo-ordinaltree-path; });

                xo-pyutil =
                  (prev.callPackage ./pkgs/xo-pyutil.nix { xo-cmake = xo-cmake;
                                                           xo-refcnt = xo-refcnt;
                                                           python = python;
                                                           pybind11 = pybind11;
                                                         }).overrideAttrs
                    (old: { src = xo-pyutil-path; });

                xo-reflect =
                  (prev.callPackage ./pkgs/xo-reflect.nix { xo-cmake = xo-cmake;
                                                            xo-subsys = xo-subsys;
                                                            xo-refcnt = xo-refcnt; }).overrideAttrs
                    (old: { src = xo-reflect-path; });

                xo-pyreflect =
                  (prev.callPackage ./pkgs/xo-pyreflect.nix { xo-cmake = xo-cmake;
                                                              xo-refcnt = xo-refcnt;
                                                              xo-pyutil = xo-pyutil;
                                                              xo-reflect = xo-reflect; }).overrideAttrs
                    (old: { src = xo-pyreflect-path; });

                xo-unit =
                  (prev.callPackage ./pkgs/xo-unit.nix { xo-cmake = xo-cmake;
                                                         xo-reflect = xo-reflect; }).overrideAttrs
                    (old: { src = ./.; });

                xo-printjson =
                  (prev.callPackage ./pkgs/xo-printjson.nix { xo-cmake = xo-cmake;
                                                              xo-reflect = xo-reflect; }).overrideAttrs
                    (old: { src = xo-printjson-path; });

                xo-pyprintjson =
                  (prev.callPackage ./pkgs/xo-pyprintjson.nix { xo-cmake = xo-cmake;
                                                                xo-pyutil = xo-pyutil;
                                                                xo-printjson = xo-printjson;
                                                                xo-pyreflect = xo-pyreflect; }).overrideAttrs
                    (old: { src = xo-pyprintjson-path; });

                xo-callback =
                  (prev.callPackage ./pkgs/xo-callback.nix { xo-cmake = xo-cmake;
                                                             xo-reflect = xo-reflect; }).overrideAttrs
                    (old: { src = xo-callback-path; });

                xo-webutil =
                  (prev.callPackage ./pkgs/xo-webutil.nix { xo-cmake = xo-cmake;
                                                            xo-reflect = xo-reflect;
                                                            xo-callback = xo-callback; }).overrideAttrs
                    (old: { src = xo-webutil-path; });

                xo-pywebutil =
                  (prev.callPackage ./pkgs/xo-pywebutil.nix { xo-cmake = xo-cmake;
                                                              xo-webutil = xo-webutil;
                                                              xo-pyutil = xo-pyutil; }).overrideAttrs
                    (old: { src = xo-pywebutil-path; });

                xo-reactor =
                  (prev.callPackage ./pkgs/xo-reactor.nix { xo-cmake = xo-cmake;
                                                            xo-randomgen = xo-randomgen;
                                                            xo-webutil = xo-webutil;
                                                            xo-printjson = xo-printjson;
                                                            xo-ordinaltree = xo-ordinaltree; }).overrideAttrs
                    (old: { src = xo-reactor-path; });

                xo-pyreactor =
                  (prev.callPackage ./pkgs/xo-pyreactor.nix { xo-cmake = xo-cmake;
                                                              xo-reactor = xo-reactor;
                                                              xo-pyutil = xo-pyutil;
                                                              xo-pyreflect = xo-pyreflect;
                                                              xo-pyprintjson = xo-pyprintjson;
                                                            }).overrideAttrs
                    (old: { src = xo-pyreactor-path; });

                xo-simulator =
                  (prev.callPackage ./pkgs/xo-simulator.nix { xo-cmake = xo-cmake;
                                                              xo-reactor = xo-reactor;
                                                            }).overrideAttrs
                    (old: { src = xo-simulator-path; });

                xo-pysimulator =
                  (prev.callPackage ./pkgs/xo-pysimulator.nix { xo-cmake = xo-cmake;
                                                                xo-simulator = xo-simulator;
                                                                xo-pyutil = xo-pyutil;
                                                                xo-pyreactor = xo-pyreactor;
                                                            }).overrideAttrs
                    (old: { src = xo-pysimulator-path; });

                xo-distribution =
                  (prev.callPackage ./pkgs/xo-distribution.nix { xo-cmake = xo-cmake;
                                                                 xo-refcnt = xo-refcnt;
                                                               }).overrideAttrs
                    (old: { src = xo-distribution-path; });

                xo-pydistribution =
                  (prev.callPackage ./pkgs/xo-pydistribution.nix { xo-cmake = xo-cmake;
                                                                   xo-distribution = xo-distribution;
                                                                   xo-pyutil = xo-pyutil;
                                                                 }).overrideAttrs
                    (old: { src = xo-pydistribution-path; });

                xo-process =
                  (prev.callPackage ./pkgs/xo-process.nix { xo-cmake = xo-cmake;
                                                            xo-simulator = xo-simulator;
                                                            xo-randomgen = xo-randomgen;
                                                          }).overrideAttrs
                    (old: { src = xo-process-path; });

                xo-pyprocess =
                  (prev.callPackage ./pkgs/xo-pyprocess.nix { xo-cmake = xo-cmake;
                                                              xo-process = xo-process;
                                                              xo-pyutil = xo-pyutil;
                                                              xo-pywebutil = xo-pywebutil;
                                                              xo-pyreactor = xo-pyreactor;
                                                            }).overrideAttrs
                    (old: { src = xo-pyprocess-path; });

                xo-statistics =
                  (prev.callPackage ./pkgs/xo-statistics.nix { xo-cmake = xo-cmake;
                                                               #xo-reactor = xo-reactor;
                                                               }).overrideAttrs
                    (old: { src = xo-statistics-path; });

                xo-kalmanfilter =
                  (prev.callPackage ./pkgs/xo-kalmanfilter.nix { xo-cmake = xo-cmake;
                                                                 xo-statistics = xo-statistics;
                                                                 xo-reactor = xo-reactor;
                                                               }).overrideAttrs
                    (old: { src = xo-kalmanfilter-path; });


                xo-pykalmanfilter =
                  (prev.callPackage ./pkgs/xo-pykalmanfilter.nix { xo-cmake = xo-cmake;
                                                                   xo-pyutil = xo-pyutil;
                                                                   xo-kalmanfilter = xo-kalmanfilter;
                                                                   xo-pyreactor = xo-pyreactor;
                                                               }).overrideAttrs
                    (old: { src = xo-pykalmanfilter-path; });

                xo-websock =
                  (prev.callPackage ./pkgs/xo-websock.nix { xo-cmake = xo-cmake;
                                                            xo-reactor = xo-reactor;
                                                               }).overrideAttrs
                    (old: { src = xo-websock-path; });

                xo-pywebsock =
                  (prev.callPackage ./pkgs/xo-pywebsock.nix { xo-cmake = xo-cmake;
                                                              xo-websock = xo-websock;
                                                              xo-pyutil = xo-pyutil;
                                                              xo-pywebutil = xo-pywebutil;
                                                            }).overrideAttrs
                    (old: { src = xo-pywebsock-path; });

                # placeholder-D

                # user environment with all xo libraries present
                xo-userenv =
                  (prev.callPackage ./pkgs/xo-userenv.nix { xo-cmake = xo-cmake;
                                                            xo-indentlog = xo-indentlog;
                                                            xo-callback = xo-callback;
                                                            xo-subsys = xo-subsys;
                                                            xo-refcnt = xo-refcnt;
                                                            xo-randomgen = xo-randomgen;
                                                            xo-ordinaltree = xo-ordinaltree;
                                                            xo-pyutil = xo-pyutil;
                                                            xo-reflect = xo-reflect;
                                                            xo-pyreflect = xo-pyreflect;
                                                            xo-unit = xo-unit;
                                                            xo-printjson = xo-printjson;
                                                            xo-pyprintjson = xo-pyprintjson;
                                                            xo-webutil = xo-webutil;
                                                            xo-pywebutil = xo-pywebutil;
                                                            xo-reactor = xo-reactor;
                                                            xo-pyreactor = xo-pyreactor;
                                                            xo-simulator = xo-simulator;
                                                            xo-pysimulator = xo-pysimulator;
                                                            xo-distribution = xo-distribution;
                                                            xo-pydistribution = xo-pydistribution;
                                                            xo-process = xo-process;
                                                            xo-pyprocess = xo-pyprocess;
                                                            xo-statistics = xo-statistics;
                                                            xo-kalmanfilter = xo-kalmanfilter;
                                                            xo-pykalmanfilter = xo-pykalmanfilter;
                                                            xo-websock = xo-websock;
                                                            xo-pywebsock = xo-pywebsock;
                                                          }).overrideAttrs(old: {});


              in
                # attrs in this set provide derivations with all overlay changes applied.
                #
                # REMINDER: need expression like
                #             packages.xo-foo = appliedOverlay.xo-foo;
                #           above to export
                {
                  xo-cmake = xo-cmake;
                  xo-indentlog = xo-indentlog;
                  xo-subsys = xo-subsys;
                  xo-refcnt = xo-refcnt;
                  xo-randomgen = xo-randomgen;
                  xo-ordinaltree = xo-ordinaltree;
                  xo-pyutil = xo-pyutil;
                  xo-reflect = xo-reflect;
                  xo-pyreflect = xo-pyreflect;
                  xo-unit = xo-unit;
                  xo-printjson = xo-printjson;
                  xo-pyprintjson = xo-pyprintjson;
                  xo-callback = xo-callback;
                  xo-webutil = xo-webutil;
                  xo-pywebutil = xo-pywebutil;
                  xo-reactor = xo-reactor;
                  xo-pyreactor = xo-pyreactor;
                  xo-simulator = xo-simulator;
                  xo-pysimulator = xo-pysimulator;
                  xo-distribution = xo-distribution;
                  xo-pydistribution = xo-pydistribution;
                  xo-process = xo-process;
                  xo-pyprocess = xo-pyprocess;
                  xo-statistics = xo-statistics;
                  xo-kalmanfilter = xo-kalmanfilter;
                  xo-pykalmanfilter = xo-pykalmanfilter;
                  xo-websock = xo-websock;
                  xo-pywebsock = xo-pywebsock;
                  # placeholder-E

                  xo-userenv = xo-userenv;

                  devShells = {
                    default = prev.mkShell.override
                      # but may need prev.clang16Stdenv instead of prev.stdenv here on macos
                      { stdenv = prev.stdenv; }

                      { packages
                        = [ python
                            pybind11
                            pythonPackages.coverage
                            pythonPackages.sphinx
                            pythonPackages.sphinx-rtd-theme
                            pythonPackages.breathe
                            # pythonPackages.pyarrow
                            boost   # really for filemerge

                            prev.llvmPackages_16.clang-unwrapped

                            prev.anki
                            prev.mesa
                            prev.egl-wayland

                            prev.emacs29
                            prev.notmuch
                            prev.emacsPackages.notmuch
                            prev.inconsolata-lgc

                            prev.doxygen
                            prev.graphviz

                            prev.ditaa
                            prev.semgrep
                            prev.ripgrep
                            prev.git
                            prev.openssh
                            prev.cmake
                            prev.gdb
                            prev.which
                            prev.man
                            prev.man-pages
                            prev.less
                            prev.tree
                            prev.nix-tree
                            prev.lcov

                            prev.arrow-cpp
                            prev.libwebsockets
                            prev.jsoncpp
                            prev.eigen
                            prev.catch2
                            prev.pkg-config
                            prev.zlib
                          ];
                      };
                  };
                });
        };

}
