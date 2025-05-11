{
  # nixpkgs dependencies
  buildFHSUserEnv, # ... other deps here

  # xo dependencies
  xo-cmake, xo-indentlog, xo-flatstring, xo-subsys, xo-refcnt, xo-ratio, xo-reflect, xo-randomgen, xo-unit,

  # other args

  # someconfigurationoption ? false
} :

buildFHSUserEnv {
  name = "xo-userenv";
  targetPkgs = pkgs: [ xo-cmake
                       xo-indentlog
                       xo-flatstring
                       xo-subsys
                       xo-refcnt
                       xo-ratio
                       xo-reflect
                       xo-randomgen
                       xo-unit
                     ];
  # runScript = ...;
  # profile = ...;
}
