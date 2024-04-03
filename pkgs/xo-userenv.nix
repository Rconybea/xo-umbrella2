{
  # nixpkgs dependencies
  buildFHSUserEnv, # ... other deps here

  # xo dependencies
  xo-cmake, xo-indentlog, xo-subsys, xo-refcnt, xo-reflect, xo-unit,

  # other args

  # someconfigurationoption ? false
} :

buildFHSUserEnv {
  name = "xo-userenv";
  targetPkgs = pkgs: [ xo-cmake
                       xo-indentlog
                       xo-subsys
                       xo-refcnt
                       xo-reflect
                       xo-unit
                     ];
  # runScript = ...;
  # profile = ...;
}
