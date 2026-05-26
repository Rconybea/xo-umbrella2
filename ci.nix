# ci.nix - minimal entry point for CI builds
# Uses nixpkgs from NIX_PATH by default (whatever the Nix daemon provides)
#
{ nixpkgs-path ? <nixpkgs> }:

let
  pkgs = import nixpkgs-path {};

  # xo.nix now returns a plain attrset (not pkgs.extend)
  xoPkgs = import ./xo.nix { inherit pkgs; };

in
  {
    inherit (xoPkgs)
      xo-cmake
      xo-indentlog
      xo-subsys
      xo-flatstring
      xo-randomgen
      xo-reflectutil
      xo-arena
      xo-facet
      xo-printable2
      xo-alloc2
      xo-gc
      xo-allocutil
      xo-refcnt
      xo-testutil
      xo-reflect
      xo-ratio
      xo-unit
      xo-callback
      xo-alloc
      xo-stringtable2
      xo-object
      xo-type
      xo-object2
      xo-procedure2
      xo-numeric
      xo-ordinaltree
      xo-tokenizer2
      xo-expression
      xo-tokenizer
      xo-reader
      xo-jit
      xo-userenv
      ;
  }
