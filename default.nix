# default.nix - xo development environment
#
# Constructs pkgs with build-from-source overlays (for local nixpkgs),
# then delegates to shells.nix for dev shell definitions.
#
# For pre-built nixpkgs (e.g. CI or binary cache), use shells.nix directly
# with a plain pkgs that has xo-overlay applied.
#
{
  # official 24.05 release  # nearly works on macos, clang17, llvm18 except for sphinx-contrib.ditaa
# on roly-chicago-24: using nixos-25.05, local nixpkgs
#  nixpkgs-path ? fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-24.11.tar.gz",
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

  # ghostty tests require ptys
  ghostty-overlay = self: super: {
    ghostty = super.ghostty.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # fish tests require ptys
  fish-overlay = self: super: {
    fish = super.fish.overrideAttrs (old: {
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
  llvm-overlay = self: super:
    # use 'super' when you want to override the terms of a package.
    # use 'self'  when pointing to an existing package
    #

    let
      llvmPackages = super.llvmPackages_18;
    in let
      clangStdenv = super.overrideCC super.stdenv super.llvmPackages_18.clang;
    in let
      jitStdenv = if super.stdenv.isDarwin then clangStdenv else super.stdenv;
    in
      { inherit llvmPackages clangStdenv jitStdenv; };

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

in
let
  pkgs = import nixpkgs-path {
    overlays = [
      amf-headers-overlay
      dejagnu-overlay
      libffi-overlay
      libqmi-overlay
      swtpm-overlay
      mailutils-overlay
      notmuch-overlay
      ghostty-overlay
      fish-overlay
#      nixgl-overlay
#      llvm-overlay
      # note: xo-overlay applied inside shells.nix
    ];
  };

in
  import ./shells.nix { inherit pkgs; }
