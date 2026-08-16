# ci.nix - minimal entry point for CI builds
# Uses nixpkgs from NIX_PATH by default (whatever the Nix daemon provides)
#
{ nixpkgs-path ? <nixpkgs>,
  overlays ? []
}:

let
  pkgs = import nixpkgs-path { inherit overlays; };

  # xo.nix now returns a plain attrset (not pkgs.extend)
  xoPkgs = import ./xo.nix { inherit pkgs; };

in
  {
    inherit (xoPkgs)
      xo-cmake
      xo-timeutil
      xo-ppsink
      xo-subsys
      xo-indentlog
      xo-randomgen
      xo-flatstring
      xo-reflectutil
      xo-arena
      xo-facet
      xo-printable2
      xo-testutil
      xo-indentlog2
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
      xo-equable2
      xo-hashable2
      xo-numeric
      xo-tokenizer2
      xo-expression2
      xo-reader2
      xo-interpreter2
      xo-allocutil
      xo-ratio
      xo-unit
      xo-pyunit
      xo-callback
      xo-alloc
      xo-object
      xo-ordinaltree
      xo-expression
      xo-pyexpression
      xo-tokenizer
      xo-reader
      xo-interpreter
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
      xo-statistics
      xo-distribution
      xo-pydistribution
      xo-simulator
      xo-pysimulator
      xo-process
      xo-pyprocess
      xo-kalmanfilter
      xo-pykalmanfilter
      xo-imgui
      xo-userenv
      xo-docs-site
      ;
  }
