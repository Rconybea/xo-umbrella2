{
  # 1. nixpkgs dependencies
  # 1.1. python
  python, pythonPackages,

  # 1.2. particular python packages
  sphinx ? pythonPackages.sphinx,
  sphinx-rtd-theme ? pythonPackages.sphinx-rtd-theme,
  breathe ? pythonPackages.breathe,

  # 1.3. document-generation packages
  #      use of makeFontsConf adapted from nixpkgs/development/libraries/gtkmm/4.x.nix
  #
  doxygen, graphviz,
  ##fontconfig,
  ##makeFontsConf,

  # 1.4. c++ build/utest chain
  stdenv, cmake, catch2, # ... other deps here

  # 2. xo dependencies
  xo-cmake, xo-indentlog

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
    name = "xo-flatstring";

    src = (fetchGit {
      url = "https://github.com/rconybea/xo-flatstring";
      version = "1.0";
      #ref = "ex1";
      #rev = "c0472c9d7e4d2c53bfb977d3182380832fe96645";
    });

    cmakeFlags = ["-DXO_CMAKE_CONFIG_EXECUTABLE=${xo-cmake}/bin/xo-cmake-config"];

    # move HOME so fontconfig can do sensible things
    buildPhase = ''
      #set -x

      echo "FONTCONFIG_FILE=$FONTCONFIG_FILE"

      #export FONTCONFIG_FILE=$fontconfig.out/etc/fonts/fonts.conf
      export HOME=$TMPDIR
      export XDG_CONFIG_HOME=$TMPDIR

      mkdir $XDG_CONFIG_HOME/fontconfig

      #grep xdg $FONTCONFIG_FILE

      #$fontconfig.out/bin/fc-cache -v

      make && make docs
      #make
    '';

    doCheck = true;
    nativeBuildInputs = [ cmake catch2
                          doxygen graphviz sphinx sphinx-rtd-theme breathe ];
    propagatedBuildInputs = [ xo-indentlog ];
  })
