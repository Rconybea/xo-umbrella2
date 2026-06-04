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
      xo-expression2
      xo-reader2
      xo-interpreter2
      xo-allocutil
      xo-alloc
      xo-ratio
      xo-unit
      xo-pyunit
      xo-callback
      xo-object
      xo-ordinaltree
      xo-expression
      xo-pyexpression
      xo-tokenizer
      xo-reader
      xo-jit
      xo-pyjit
      xo-webutil
      xo-pywebutil
      xo-printjson
      xo-pyprintjson
      xo-reactor
      xo-pyreactor
      xo-websock
      xo-pywebsock
      xo-userenv
      ;
  }
