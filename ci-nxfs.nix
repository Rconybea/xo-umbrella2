# Necessary overlays for local nixpkgs clone built from source
# To use vanilla <nixpkgs> can use ci.nix directly.
#
import ./ci.nix {
  nixpkgs-path = ../nixpkgs;
  overlays = import ./overlays.nix;
}
