{
  # dependencies
  stdenv, cmake, # ... other deps here

  # args

  #   attrset for fetching source code.
  #    { type, owner, repo, ref }
  #
  #   e.g. type="github", owner="rconybea", repo="cmake-examples", ref="ex1b"
  #
  #   see [[../flake.nix]]
  #
  #cmake-examples-ex1-path

  # someconfigurationoption ? false
} :

stdenv.mkDerivation (finalattrs:
  {
    name = "xo-cmake";

#    src = cmake-examples-ex1-path;

    src = (fetchGit {
      url = "https://github.com/rconybea/xo-cmake";
      version = "1.0";
      #ref = "ex1";
      #rev = "c0472c9d7e4d2c53bfb977d3182380832fe96645";
    });

    nativeBuildInputs = [ cmake ];

#    installPhase = ''
#      mkdir -p $out
#      echo 'This project intentionally has no install phase'
#    '';
  })
