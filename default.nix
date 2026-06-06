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
  pkgs = import nixpkgs-path {
    # overlays for nix-from-scratch
    overlays = import ./overlays.nix;
  };

in
  import ./shells.nix { inherit pkgs; }
