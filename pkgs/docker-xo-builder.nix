# NOTE: Looks like need container with /etc/passwd to use git submodules.
#       Get error:
#         No user exists for uid 0
#       that we don't see when invoking 'git clone' directly.
#
#       See ~/proj/org-howto/articles/2026/06/forgejo-docker-container-registry.org setup
#       See ~/proj/docker-nix-builder for somewhat more full-featured setup attempt
#       See ~/proj/nix/docker.nix for the make-me-one-with-everything version

{
  dockerTools,

  # miscellaneous
  tree, which,

  # deps for working with git
  git, openssh, curl, wget, cacert,

  # xo deps
  eigen, libwebsockets, jsoncpp, libunwind, elfutils,

  # archives
  gnutar, gzip,

  # packages
  pkg-config,

  # python toolchain
  python3Packages,
  sphinx ? python3Packages.sphinx,
  sphinx-rtd-theme ? python3Packages.sphinx-rtd-theme,
  breathe ? python3Packages.breathe,

  # c++ toolchain
  catch2, cmake, gnumake, gcc, doxygen, graphviz,

  # base platform stuff
  gawk, gnused, gnugrep, patch, xz, bzip2,
  diffutils, findutils, binutils, bashInteractive, bash, coreutils, lib
} :

let
  users = {
    # root user.
    root = {
      uid = 0;
      gid = 0;

      shell = "${bashInteractive}/bin/bash";
      #shell = "${bashInteractive}/bin/bash";
      # TODO: probably move to /root
      home = "/";
      groups = [ "root" ];
      description = "system administrator";
    };
  };

  groups = {
    root.gid = 0;
    #nixbld.gid = 30000;
    #nobody.gid = 65534;
  };

  # convert from user struct to row in /etc/passwd
  user2passwd = (key: { uid, gid, home, description, shell, groups}: "${key}:x:${toString uid}:${toString gid}:${description}:${home}:${shell}");

  # convert from user struct to row in /etc/shadow
  user2shadow = (key: { uid, gid, home, description, shell, groups}: "${key}:!:1::::::");

  # contents of /etc/passwd
  passwd = (lib.concatStringsSep "\n" (lib.attrValues (lib.mapAttrs user2passwd users)));

  # contents of /etc/shadow
  shadow = (lib.concatStringsSep "\n" (lib.attrValues (lib.mapAttrs user2shadow users)));

  # figure out which users belong to which groups
  #
  # group2member_map :: {group, [user]}
  #
  group2member_map = (
    let
      # mappings :: [{user, group}]
      #
      # e.g. [ {user="nixbld1"; group="nixbld"; }, {user="nixbld2"; group="nixbld";}, ... ]
      #
      mappings = (
        builtins.foldl'
          (acc: user:
            let
              groups = users.${user}.groups or [ ];
            in
              acc ++ (map (group: { inherit user group; }) groups)
          )
          [ ]
          (lib.attrNames users)
      );
    in
      (
        builtins.foldl'
          (
            # v :: {user, group}
            acc: v: acc // { ${v.group} = acc.${v.group} or [ ] ++ [ v.user ]; }
          )
          { }
          mappings)
  );

  # group2group :: gname -> gid -> groupline
  #
  # e.g. "nixbld" -> 30000 -> "nixbld:x:30000:nixbld1,nixbld2"
  #
  group2group =
    (key : { gid }:
      let
        # member_list :: [user]
        member_list = group2member_map.${key} or [ ];
        memberlist_str = lib.concatStringsSep "," member_list;
      in
        "${key}:x:${toString gid}:${memberlist_str}");

  # contents of /etc/group
  group = (lib.concatStringsSep "\n" (lib.attrValues (lib.mapAttrs group2group groups)));

in

dockerTools.buildLayeredImage {
  name = "docker-xo-builder";
  tag = "v2";
  created = "now";   # warning: breaks deterministic output!

  # use (lib.getDev foo) on a library package foo
  # to request developer support,  for example .cmake files
  #
  contents = [ tree
               which

               git
               openssh
               curl
               wget
               cacert

               sphinx
               sphinx-rtd-theme
               breathe

               (lib.getDev python3Packages.pybind11)
               python3Packages.python

               doxygen
               graphviz

               (lib.getDev eigen)
               (lib.getDev libwebsockets)
               (lib.getDev jsoncpp)
               (lib.getDev libunwind)
               (lib.getDev elfutils)

               pkg-config

               gnutar
               gzip
               xz
               bzip2

               catch2
               cmake
               gnumake
               gcc

               patch
               gawk
               gnused
               gnugrep
               diffutils
               findutils
               binutils
               bashInteractive
               bash
               coreutils ];

  enableFakechroot = true;

  fakeRootCommands = ''
    mkdir -p /etc
    mkdir -p /var

    mkdir -p /usr/bin
    ln -s ${coreutils}/bin/env /usr/bin/env

    #mkdir -p /.config/nix
    #echo "experimental-features = nix-command flakes" > .config/nix/nix.conf

    echo "${passwd}" > /etc/passwd
    echo "${shadow}" > /etc/shadow
    echo "${group}" > /etc/group

    #mkdir -p /tmp
    #mkdir -p /var/tmp

    #chmod 1777 /tmp
    #chmod 1777 /var/tmp
  '';

  config = {
    # note: nix provider docker.nix that prepares do-anything-nixy docker image.
    #       That version takes measures to allow nix cmds to override settings here:
    #       1. uses .nix-profile/bin/bash for shell
    #       2. puts certs under /nix/var/nix/profiles/default/etc/...
    #
    Cmd = [ "/bin/bash" ];
    Env = [
      "SSL_CERT_FILE=/etc/ssl/certs/ca-bundle.crt"
      "PKG_CONFIG_PATH=/lib/pkgconfig:/share/pkgconfig"
    ];
  };

}
