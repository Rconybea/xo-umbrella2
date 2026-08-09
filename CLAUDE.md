# xo-umbrella2

## Agent skills

### Issue tracker

Issues and specs live as markdown under `.xo-backlog/<feature-slug>/` (spec at
`spec.md`, tickets under `issues/NN-<slug>.md`). `.xo-backlog` is a symlink to a
separate, shared repo — commit and push changes there, not here. See
`docs/agents/issue-tracker.md`.

**Read `.xo-backlog/CONVENTIONS.md` before writing a ticket or verifying a
change.** It covers how claims in tickets must be grounded, the `xo-deps`
queries for checking dependency claims, and the build/test/nix recipe for this
tree — including the flags whose absence has silently hidden breakage
(`--with-examples`, `-q` rather than `>/dev/null`).

**Start with `xo-sdlc --milestones`** for what large effort is in flight and how
far along it is. A milestone is work too big for one ticket; its progress is a
query over tickets carrying a `Milestone:` line, so it cannot go stale. See the
Milestones section of `docs/agents/issue-tracker.md`.

### Domain docs

Single-context — `CONTEXT.md` + `docs/adr/` at the repo root, created lazily by
`/domain-modeling` when terms or decisions get resolved. See `docs/agents/domain.md`.
