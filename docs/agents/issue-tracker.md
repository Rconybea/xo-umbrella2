# Issue tracker: Local Markdown

Issues and specs (you may know a spec as a PRD) for xo work live as markdown files
under `.xo-backlog/`.

`.xo-backlog` is a **symlink to a separate repo** (`github.com/Rconybea/xo-backlog`,
cloned alongside this one), not a directory in this repo. It holds meta content about
how the xo subsystems evolve, so it is shared: every xo workspace on every host --
`xo-umbrella2`, `xo`, and any other build directory -- symlinks to the same clone, and
tickets written from one are visible from all of them. Make the symlink relative
(`ln -s ../xo-backlog .xo-backlog`) so it survives differing home directories across
hosts.

Consequences worth knowing: changes there are committed and pushed in **that** repo,
not this one; and because `.xo-backlog` is gitignored here, `git clean -fdx` removes
the symlink (harmless -- recreate it; the content lives outside this tree).

## Conventions

- One feature per directory: `.xo-backlog/<feature-slug>/`
- The spec is `.xo-backlog/<feature-slug>/spec.md`
- Implementation issues are one file per ticket at `.xo-backlog/<feature-slug>/issues/<NN>-<slug>.md`, numbered from `01` — never a single combined tickets file
- Triage state is recorded as a `Status:` line near the top of each issue file (see `triage-labels.md` for the role strings)
- Comments and conversation history append to the bottom of the file under a `## Comments` heading

## Milestones

A **milestone** is a unit of work too large for one ticket, not worked on
directly: it completes when the right set of ordinary tickets are done.

- Milestone file: `.xo-backlog/milestones/<slug>.md`, with a `Status: open|done`
  line. It holds the reasoning — the design question, the shape of done, links
  to precedents.
- **A milestone never lists its tickets.** Each contributing ticket carries a
  `Milestone: <slug>` line near the top (comma-separated for more than one,
  like `Blocked by:`), and the set is a query over those.
- Query with `xo-sdlc --milestones` (open ones, with progress) or
  `xo-sdlc --milestone=<slug>` (per-ticket detail). `--backlog=DIR` points at a
  different sandbox's backlog; `--all` includes closed milestones.
- `xo-sdlc --tickets` lists open tickets across the whole backlog, annotating
  each with its milestone; `--all` includes closed ones. It shares the
  done-predicate with the milestone query, so the two views cannot disagree —
  worth preserving, since an ad-hoc `grep` for `Status:` will quietly use a
  different notion of "done".

Two properties worth preserving if this is ever reworked:

**Progress is derived, closing is deliberate.** The query counts tickets; you
still set `Status: done` by hand, because a milestone usually has criteria no
ticket covers, and because the verification is the valuable half of closing.

**The linkage lives in the tickets, not the milestone.** A hand-maintained list
in the milestone file would drift the first time someone filed a ticket and
forgot to add it — the same failure this project has hit repeatedly with
`subsystem-edges`, `Config.cmake.in` and `pkgs/*.nix`, each fixed by making the
derived thing derived.

## When a skill says "publish to the issue tracker"

Create a new file under `.xo-backlog/<feature-slug>/` (creating the directory if needed).

## When a skill says "fetch the relevant ticket"

Read the file at the referenced path. The user will normally pass the path or the issue number directly.

## Wayfinding operations

Used by `/wayfinder`. The **map** is a file with one **child** file per ticket.

- **Map**: `.xo-backlog/<effort>/map.md` — the Notes / Decisions-so-far / Fog body.
- **Child ticket**: `.xo-backlog/<effort>/issues/NN-<slug>.md`, numbered from `01`, with the question in the body. A `Type:` line records the ticket type (`research`/`prototype`/`grilling`/`task`); a `Status:` line records `claimed`/`resolved`.
- **Blocking**: a `Blocked by: NN, NN` line near the top. A ticket is unblocked when every file it lists is `resolved`.
- **Frontier**: scan `.xo-backlog/<effort>/issues/` for files that are open, unblocked, and unclaimed; first by number wins.
- **Claim**: set `Status: claimed` and save before any work.
- **Resolve**: append the answer under an `## Answer` heading, set `Status: resolved`, then append a context pointer (gist + link) to the map's Decisions-so-far in `map.md`.
