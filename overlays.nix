let
  # this approach (overlays) is effective, but has super wide cross-section,
  # since absolutely everything has to be rebuilt from source
  #

  amf-headers-overlay = self: super: {
    amf-headers = super.amf-headers.overrideAttrs (old: {
      src = self.fetchFromGitHub {
        # nixpkgs sha256 stale for version 1.4.36 asof 4sep2025
        owner = "GPUOpen-LibrariesAndSDKs";
        repo = "AMF";
        rev = "v${old.version}";
        sha256 = "sha256-0PgWEq+329/EhI0/CgPsCkJ4CiTsFe56w2O+AcjVUdc=";
        #sha256 = "sha256-0PgWEq+329/EhI0/CgPsCkJ4CiTsFe56w2O+AcjVUdc=";
        #sha256 = "sha256-u6gvdc1acemd01TO5EbuF3H7HkEJX4GUx73xCo71yPY=";
      };});
  };

  # complains about 'no more ptys'
  dejagnu-overlay = self: super: {
    dejagnu = super.dejagnu.overrideAttrs (old: {
      doCheck = false;
    });
  };

  swtpm-overlay = self: super: {
    swtpm = super.swtpm.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # libffi tests run, but only if sandbox disabled
  libffi-overlay = self: super: {
    libffi = super.libffi.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # libqmi test seems to stall on ubuntu
  libqmi-overlay = self: super: {
    libqmi = super.libqmi.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # fop 2.8 source moved.
  # mirror://apache path 404s because Apache relocated old releases into source/.
  # Keep nixpkgs' 2.8 + repoint to permanent archive.
  #
  fop-overlay = final: prev: {
    fop = prev.fop.overrideAttrs (old: {
      src = final.fetchurl {
        url = "https://archive.apache.org/dist/xmlgraphics/fop/source/fop-2.8-src.tar.gz";
        hash = "sha256-b7Av17wu6Ar/npKOiwYqzlvBFSIuXTpqTacM1sxtBvc=";
      };
    });
  };

  # nodejs test-runner-output test fails.
  # advice is that this test is notoriously flaky, and sibling
  # tests are already disabled in nixpkgs 25.05
  #
  node-overlay = final: prev: {
    nodejs_22 = prev.nodejs_22.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # salsa upstream/1.36 tag was re-pointed and dropped `wrapawk`
  # (release-tarball-only, never committed).
  # Use the canonical Debian orig tarball from snapshot.debian.org:
  # permanent + content-addressed by file hash, so can't drift.
  #
  fakeroot-overlay = final: prev: {
    fakeroot = prev.fakeroot.overrideAttrs (old: {
      src = final.fetchzip {
        url = "https://snapshot.debian.org/file/ea895c6632fcf1b38cc84987d1b4daf833ffd430";
        extension = "tar.gz";   # snapshot URLs carry no extension, we must apply ourselves
        hash = "sha256-0vM8SsJ+uqp2kAot0/NF/8e/YJX81rlnEdXdgLbyRhc=";
      };
    });
  };

  # tests excruciatingly slow
  mailutils-overlay = self: super: {
    mailutils = super.mailutils.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # 2 tests fail with 25.05
  notmuch-overlay = self: super: {
    notmuch = super.notmuch.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # ghostty tests require ptys
  ghostty-overlay = self: super: {
    ghostty = super.ghostty.overrideAttrs (old: {
      doCheck = false;
    });
  };

  # fish tests require ptys
  fish-overlay = self: super: {
    fish = super.fish.overrideAttrs (old: {
      doCheck = false;
    });
  };

in
[
  amf-headers-overlay
  dejagnu-overlay
  libffi-overlay
  libqmi-overlay
  fop-overlay
  node-overlay
  fakeroot-overlay
  lato-overlay
  plotly-overlay
  flasgger-overlay
  igraph-overlay
  swtpm-overlay
  mailutils-overlay
  notmuch-overlay
  ghostty-overlay
]
