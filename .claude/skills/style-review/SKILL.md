---
name: style-review
description: Review (and optionally fix) changed C++ code against docs/coding-style.rst -- symbol qualification (individual using-declarations collected in one place, no `using namespace`, no inline xo:: qualifier noise) and include brackets/order. Use when asked to review or fix style on staged changes, a branch, or named files.
---

# style-review

Check changed C++ code against the XO coding style, and fix it when asked.

## Authority

`docs/coding-style.rst` is the spec.  Read it at the start of every run --
it changes, and this skill does not restate its rules so that it cannot
drift from them.  Where this file and the doc disagree, the doc wins.

## Scope

Only code the change touches -- never the whole tree.  Legacy code outside
the diff is not the subject; mention a pre-existing violation only if it is
on a line the diff changes.

Pick the diff from the request:

| request | diff |
|---|---|
| "staged" (default when nothing is staged-vs-unstaged ambiguous) | `git diff --cached` |
| "unstaged" / "working tree" | `git diff` |
| a base ("since main", "since abc123") | `git diff <base>...HEAD` |
| named files | those files, but still report only changed lines unless asked for the whole file |

Only `.cpp` / `.hpp` files under `xo-*/`.  Skip generated files (anything
under a build directory, `*_genfacet*` outputs).

## Mode

- **review** (default): report findings; change nothing.
- **fix**: only when the request says fix / apply.  Apply the fixes, then
  build and test (below), then show the diff of what changed.  Never commit.

## What to check

From `docs/coding-style.rst`:

1. **Symbol qualification**
   - `using namespace ...` introduced or touched -> finding.
   - Inline-qualified uses (`xo::foo::bar`) in changed lines -> finding,
     unless an exemption applies (see below).  The same qualifier repeated
     for the same symbol is the typical case.
   - A `using` declaration in the wrong place:
     - .cpp: not inside the top-level `namespace xo {` (or `namespace ut {`
       in a unit test), or scattered rather than in the one block;
     - .hpp: at namespace scope (leaks into includers) -> must move to the
       top of the function body, or become a class-scope alias at the top
       of the class;
     - class: aliases not collected at the top of the class.
2. **Includes** (brackets, no `../`, order) for `#include` lines the diff
   adds or moves.  For dependency order look the subsystems up in
   `.build/dot-clang-format` (`grep -n "xo/<sub>/" .build/dot-clang-format`;
   lower priority sorts first) -- never guess.

### Exemptions -- leave qualified, do not report

- Where C++ requires the qualified name: a template specialization's
  argument at namespace scope (`Prettifier<xo::json::json_open>`),
  out-of-line definitions (`Foo::bar(..)`), a qualified base class or
  friend declaration where no using-declaration can reach.
- Deliberate disambiguation or ADL suppression -- qualified on purpose.
  If unsure, report it as a question, not a finding.
- Text produced by macro expansion (e.g. `REFLECT_MEMBER`).
- `std::` and other non-XO namespaces: the doc's examples are XO
  namespaces.  Report `std::` only if the doc is changed to cover it.

## Grounding: when unsure how a name resolves

Use clang's AST rather than reading text (clang 18 in the builder shell,
compile flags from the clang tree):

```bash
cd "$(xo-sdlc --umbrella2)"
nix-shell ci-nxfs.nix -A xo-builder-shell --run \
  'clang-query -p .build-clang -c "set output diag" \
     -c "match usingDirectiveDecl(isExpansionInMainFile())" \
     -c "match declRefExpr(isExpansionInMainFile(), has(nestedNameSpecifier()))" \
     -c "match usingDecl(isExpansionInMainFile())" \
     <file.cpp>'
```

- `usingDirectiveDecl` -- each `using namespace`.
- `declRefExpr(... has(nestedNameSpecifier()))` -- each inline-qualified
  reference to a function or variable.  Qualified TYPES are not
  `declRefExpr`s; this matcher misses them -- read those from the source.
- `usingDecl` -- each using-declaration, with its location (placement).

For a .hpp, run it on a .cpp that includes the header and filter the
results to the header's path.  NOT the JSON dump (`-ast-dump=json`): it
drops the qualifier from references, so it cannot tell `a::b::f()` from a
bare `f()`.

## Fixing

For each finding in fix mode:

- **Inline qualification in a .cpp**: add `using xo::a::b::name;` to the
  scope's one block (create it at the top of `namespace xo {` /
  `namespace ut {` if absent; keep it grouped with the existing usings),
  and write the bare name at every use in the file -- not only the changed
  line, so the file is consistent.
- **In a class**: add `using name = xo::a::b::name;` with the class's other
  aliases at its top.
- **In a function defined in a .hpp**: add `using xo::a::b::name;` at the
  top of the function body.
- **`using namespace`**: replace with one using-declaration per symbol the
  file actually uses from that namespace (find them with clang-query or by
  building with the directive removed and reading the errors).

Do NOT fix -- report instead, with the reason -- when:

- two symbols with the same bare name from different namespaces would
  collide in the scope;
- the bare name could change overload resolution or ADL (an unqualified
  call whose argument types live in another namespace that also declares
  that name);
- the line is not in the diff and the user did not ask for whole-file.

### After fixing

Build and test what was touched, and report the results verbatim:

```bash
cd "$(xo-sdlc --umbrella2)"
cmake --build .build -j16 2>&1 | grep -E ' error|warning:'
ctest --test-dir .build -j8 2>&1 | grep -E 'tests passed|Failed'
```

A fix that does not build, or turns a test red, is reverted and reported
as a finding instead.

## Report

One entry per finding, most important first:

```
<file>:<line>  <rule>  -- <what is wrong>
    fix: <the change>          (or: left as is -- <exemption / reason>)
```

Then a one-line summary: N findings, M fixed (fix mode), K left with
reasons.  Always name the file with a line number.  If there are no
findings, say so plainly.
