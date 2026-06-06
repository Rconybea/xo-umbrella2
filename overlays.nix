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
  dejagnu-overlay
  libffi-overlay
  libqmi-overlay
  swtpm-overlay
  mailutils-overlay
  notmuch-overlay
  ghostty-overlay
  fish-overlay
]
