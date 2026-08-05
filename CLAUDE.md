# xo-umbrella2

## Agent skills

### Issue tracker

Issues and specs live as markdown under `.xo-backlog/<feature-slug>/` (spec at
`spec.md`, tickets under `issues/NN-<slug>.md`). `.xo-backlog` is a symlink to a
separate, shared repo — commit and push changes there, not here. See
`docs/agents/issue-tracker.md`.

### Domain docs

Single-context — `CONTEXT.md` + `docs/adr/` at the repo root, created lazily by
`/domain-modeling` when terms or decisions get resolved. See `docs/agents/domain.md`.
