# ci.nix - minimal entry point for CI builds
# Uses nixpkgs from NIX_PATH by default (whatever the Nix daemon provides)
#
{ nixpkgs-path ? <nixpkgs> }:

let
  pkgs = import nixpkgs-path {};

  xoPkgs = import ./xo.nix { inherit pkgs; };

in
  {
    inherit (xoPkgs)
      xo-cmake
      xo-indentlog
      xo-reflectutil
      xo-randomgen
      xo-arena
      xo-facet
      xo-allocutil
      xo-refcnt
      xo-subsys
      xo-testutil
      xo-flatstring
      xo-reflect
      xo-ratio
      xo-unit
      xo-callback
      xo-printable2
      xo-alloc
      xo-alloc2
      xo-stringtable2
      xo-gc
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
