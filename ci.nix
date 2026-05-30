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
      xo-testutil
      xo-alloc2
      xo-refcnt
      xo-pyutil
      xo-reflect
      xo-pyreflect
      xo-stringtable2
      xo-object2
      xo-gc
      xo-type
      xo-procedure2
      xo-numeric
      xo-tokenizer2
      xo-allocutil
      xo-alloc
      xo-ratio
      xo-unit
      xo-callback
      xo-object
      xo-ordinaltree
      xo-expression
      xo-tokenizer
      xo-reader
      xo-jit
      xo-userenv
      ;
  }
