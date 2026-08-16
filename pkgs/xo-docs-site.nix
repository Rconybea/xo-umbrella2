# xo-docs-site.nix
#
# Assembles the published documentation site from subsystem packages that already
# build their own docs.  Nothing here runs sphinx: each subsystem package with
# buildDocs = true installs a self-contained html tree under
# share/doc/<PROJECT_NAME>/html (see xo_docdir_sphinx_config in
# xo-cmake/cmake/xo_macros/xo_cxx.cmake).  This just gathers them behind stable URLs
# and generates a landing page.
#
#   $out/index.html    generated landing page
#   $out/xo-unit       -> symlink to that package's html tree
#   $out/xo-arena      -> ...
#
# Symlinks rather than copies: nix records the store references, so a gc root on this
# derivation keeps every doc tree it serves alive, and republishing costs no disk.
#
# The subsystem list is NOT written here.  xo.nix passes every package carrying
# buildDocs = true, so the published index cannot drift from the set that actually
# builds docs -- adding docs to a subsystem publishes them, with no second list to
# update.  A package that claims buildDocs but installs no tree fails the build rather
# than quietly going missing from the site.
#
{ lib, runCommand, writeText, subsystems }:

let
  names = lib.attrNames subsystems;

  # The URL uses the nix attribute name (xo-unit), which matches the satellite repo.
  # The glob resolves the cmake project name (xo_unit), which does not.
  linkOne = name: drv: ''
    set -- ${drv}/share/doc/*/html
    if [ "$#" -ne 1 ] || [ ! -f "$1/index.html" ]; then
      echo "xo-docs-site: ${name} sets buildDocs = true but installed no doc tree" >&2
      echo "  expected exactly one match for ${drv}/share/doc/*/html/index.html" >&2
      exit 1
    fi
    ln -s "$1" "$out/${name}"
  '';

  indexPage = writeText "xo-docs-index.html" ''
    <!doctype html>
    <html lang="en">
    <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1"/>
    <title>XO documentation</title>
    <style>
      body { max-width: 46rem; margin: 3rem auto; padding: 0 1.5rem;
             font-family: system-ui, sans-serif; line-height: 1.5; color: #222; }
      h1 { font-size: 1.6rem; margin-bottom: 0.25rem; }
      p.tagline { color: #555; margin-top: 0; }
      ul.subsystems { list-style: none; padding: 0;
                      display: grid; grid-template-columns: repeat(auto-fill, minmax(12rem, 1fr));
                      gap: 0.5rem; }
      ul.subsystems a { display: block; padding: 0.5rem 0.75rem; border: 1px solid #ddd;
                        border-radius: 4px; text-decoration: none; color: #0b5; font-weight: 600; }
      ul.subsystems a:hover { background: #f6f6f6; }
      footer { margin-top: 3rem; color: #777; font-size: 0.9rem; }
    </style>
    </head>
    <body>
    <h1>XO documentation</h1>
    <p class="tagline">Integrated C++ libraries for event-based simulation.</p>

    <p>XO is a collection of loosely coupled subsystems, each with its own repository and
    its own documentation.  Pick the one you depend on:</p>

    <ul class="subsystems">
    ${lib.concatMapStrings (name: ''<li><a href="${name}/">${name}</a></li>
    '') names}
    </ul>

    <footer>
    Generated from the subsystem packages built by CI.  A subsystem appears here once its
    package sets <code>buildDocs = true</code>.
    </footer>
    </body>
    </html>
  '';
in

runCommand "xo-docs-site" { } ''
  mkdir -p "$out"
  ${lib.concatStrings (lib.mapAttrsToList linkOne subsystems)}
  cp ${indexPage} "$out/index.html"
''
