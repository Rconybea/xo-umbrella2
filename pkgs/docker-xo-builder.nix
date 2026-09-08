# NOTE: Looks like need container with /etc/passwd to use git submodules.
#       Get error:
#         No user exists for uid 0
#       that we don't see when invoking 'git clone' directly.
#
#       See ~/proj/org-howto/articles/2026/06/forgejo-docker-container-registry.org setup
#       See ~/proj/docker-nix-builder for somewhat more full-featured setup attempt
#       See ~/proj/nix/docker.nix for the make-me-one-with-everything version
#
# To build+deploy:
#    $ docker image prune -f   # optional hygiene
#    $ nix-build -A xo.docker-xo-builder   # builds container
#    $ docker load <$(readlink -f result)  # load into docker
#
#  ONE image, two registries -- tag it twice, push it twice.  Same image id in
#  both, so forgejo and github CI run the identical builder.
#
#    # forgejo (vpn1).  The runner picks the image by label, see
#    # /etc/forgejo-runner/config.yaml:
#    #     labels: - "xo-builder:docker://conybeare.us/roland/docker-xo-builder:v2"
#    $ docker image tag docker-xo-builder:v2 conybeare.us/roland/docker-xo-builder:v2
#    $ docker image push conybeare.us/roland/docker-xo-builder:v2
#
#    # github.  Named in the workflow itself: container: image: ghcr.io/...
#    # Needs a credential with write:packages:
#    #     gh auth refresh -h github.com -s write:packages
#    #     gh auth token | docker login ghcr.io -u rconybea --password-stdin
#    $ docker image tag docker-xo-builder:v2 ghcr.io/rconybea/docker-xo-builder:v2
#    $ docker image push ghcr.io/rconybea/docker-xo-builder:v2
#
# Then on the forgejo CI host:
#    $ docker pull conybeare.us/roland/docker-xo-builder:v2
#    $ docker image prune -f
# Will be used on next CI build.  github runners pull ghcr per-run, no step needed.

{
  dockerTools,

  # miscellaneous
  tree, which,

  # deps for working with git
  git, openssh, curl, wget, cacert,

  # xo deps
  llvm, eigen, libwebsockets, jsoncpp, cli11, libunwind, elfutils, replxx, libbsd,

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
  catch2, cmake, gnumake, gcc, clang, doxygen, graphviz,

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

               (lib.getDev llvm)
               (lib.getLib llvm)
               (lib.getDev eigen)
               (lib.getDev libwebsockets)
               (lib.getDev jsoncpp)
               (lib.getDev cli11)
               (lib.getLib cli11)
               (lib.getDev libunwind)
               (lib.getLib libunwind)
               (lib.getDev elfutils)
               (lib.getLib elfutils)
               (lib.getDev replxx)
               (lib.getLib replxx)
               (lib.getDev libbsd)
               (lib.getLib libbsd)

               pkg-config

               gnutar
               gzip
               xz
               bzip2

               catch2
               cmake
               gnumake
               gcc      # default gcc ahead of clang.
               clang

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

    mkdir -p /tmp
    mkdir -p /var/tmp

    chmod 1777 /tmp
    chmod 1777 /var/tmp
  '';

  config = {
    # note: nix provider docker.nix that prepares do-anything-nixy docker image.
    #       That version takes measures to allow nix cmds to override settings here:
    #       1. uses .nix-profile/bin/bash for shell
    #       2. puts certs under /nix/var/nix/profiles/default/etc/...
    #
    # Provenance travels WITH the image, rather than living only in a registry's
    # database.  ghcr reads image.source to link a package to its repository,
    # and that link governs package permissions -- so a stale one points access
    # control at the wrong repo.  The package was linked to Rconybea/docker-xo-builder,
    # which no longer builds it; this file does.
    #
    # `docker image inspect <image> --format '{{json .Config.Labels}}'` answers
    # "where does this come from?" without asking a registry.
    Labels = {
      "org.opencontainers.image.source" = "https://github.com/Rconybea/xo-umbrella2";
      "org.opencontainers.image.description" = "xo build environment: gcc, clang, cmake, nix, and the xo third-party deps";
      "org.opencontainers.image.title" = "docker-xo-builder";
    };

    Cmd = [ "/bin/bash" ];
    Env = [
      "SSL_CERT_FILE=/etc/ssl/certs/ca-bundle.crt"
      "PKG_CONFIG_PATH=/lib/pkgconfig:/share/pkgconfig"
      "NIX_CFLAGS_COMPILE_${gcc.suffixSalt}=-isystem /include"
      "NIX_LDFLAGS_${gcc.suffixSalt}=-L/lib"
    ];
  };

}
