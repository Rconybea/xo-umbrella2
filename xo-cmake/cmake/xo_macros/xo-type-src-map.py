#!/usr/bin/env python3
"""xo-type-src-map - map each C++ type a subsystem defines to its source location.

Writes json:

    {"format": "xo-type-src-map/1",
     "subsystem": "xo-websock",
     "types": {"xo::web::UrlRouter": {"file": "xo-websock/include/xo/websock/UrlRouter.hpp",
                                      "line": 46},
               ...}}

Keys are fully qualified names, spelled as xo::reflect::type_name<T> spells
them (so a printer's reflected canonical_name looks up directly; a template is
keyed by its bare name).  Files are relative to --repo-root.

How: for every translation unit in the compile database whose source is under
--source-dir (library, utest, example), run clang with that TU's own flags:

    clang++ <flags> -fsyntax-only -Xclang -ast-dump -Xclang -ast-dump-filter=<filter> <tu>

clang dumps each outermost declaration whose qualified name contains the
filter, headed "Dumping <qualified name>:", with nesting in the "|-" / "`-"
tree prefix; qualified names are rebuilt by walking it.  Keeps only types
DEFINED under --source-dir: a subsystem's map holds its own types
(.xo-backlog/xo-websock/issues/12, scope B).

Skipped, by decision or because type_name cannot name them usefully:
anonymous namespaces; types defined inside functions.  Explicit / partial
specializations are not separate entries: a specialization looks up by its
template's bare name.
"""

import argparse
import concurrent.futures
import json
import os
import re
import shlex
import subprocess
import sys

FORMAT = 'xo-type-src-map/1'

# a dump node: tree prefix, kind, address
NODE_RE = re.compile(r'^((?:[| ] )*[|`]-)?(\w+) (0x[0-9a-f]+)')
# every file mentioned on a line, IN ORDER.  clang prints a location's file
# only when it differs from the last one printed ANYWHERE -- inside the <range>,
# or after it (a decl's name in another file than its range start) -- so every
# mention must be followed, not just the ones after '<'
FILE_RE = re.compile(r'(/[^:\s<>,\']+):\d+:\d+')
# record definition: "... class Foo definition"
RECORD_DEF_RE = re.compile(r' (?:class|struct|union) (\w+) definition')
RECORD_ANY_RE = re.compile(r' (?:class|struct|union) (\w+)')
# location of the decl's NAME: "> line:346:15" or "> col:15" (same line as range start)
NAMELOC_RE = re.compile(r'> (?:line:(\d+):\d+|col:\d+)')
RANGE_START_RE = re.compile(r'<(?:/[^:>,]+:)?(?:line:)?(\d+):\d+')
PARENT_RE = re.compile(r' parent (0x[0-9a-f]+)')
# a quoted type string, e.g. 'std::vector<int>'
QUOTED_RE = re.compile(r"'[^']*'")
ENUM_RE = re.compile(r'EnumDecl 0x[0-9a-f]+ .*?(?: class| struct)? (\w+)(?: \'[^\']*\')?$')

FUNCTION_KINDS = {'FunctionDecl', 'CXXMethodDecl', 'CXXConstructorDecl',
                  'CXXDestructorDecl', 'CXXConversionDecl', 'LambdaExpr',
                  'BlockDecl', 'FunctionTemplateDecl'}

ANON = '(anonymous namespace)'


# gcc's compiler-internal headers (x86 intrinsics, limits.h, ..):
# .../lib/gcc/<triple>/<version>/include{,-fixed}.  clang has its own; given
# gcc's too it fails (e.g. "conflicting types for '_mm_prefetch'" in
# xmmintrin.h).  Only where a build lists them explicitly (nix: xo-kalmanfilter)
GCC_INTERNAL_INCLUDE_RE = re.compile(r'/lib/gcc/[^/]+/[^/]+/include(-fixed)?/?$')


def tu_flags(entry):
    """compiler arguments of @p entry, minus the compiler, -c, -o <x>, the
    source, and gcc's internal include directories
    """
    args = entry.get('arguments') or shlex.split(entry['command'])
    out, skip = [], False

    i = 1
    while i < len(args):
        a = args[i]
        i += 1
        if a == '-o':
            i += 1
            continue
        if a in ('-c', entry['file']):
            continue
        if a in ('-isystem', '-I', '-idirafter') and i < len(args) \
           and GCC_INTERNAL_INCLUDE_RE.search(args[i]):
            i += 1
            continue
        if a.startswith(('-isystem', '-I')) and GCC_INTERNAL_INCLUDE_RE.search(a[2:] if a.startswith('-I') else a[8:]):
            continue
        out.append(a)

    return out


def dump_tu(entry, clang, name_filter):
    """run clang's filtered AST dump on one TU; (returncode, stdout, stderr)"""
    cmd = [clang] + tu_flags(entry) + [
        '-fsyntax-only',
        # the compile database is gcc's: tolerate what clang does not know
        '-Wno-unknown-warning-option', '-Qunused-arguments',
        '-Xclang', '-ast-dump',
        '-Xclang', '-ast-dump-filter=' + name_filter,
        entry['file']]
    r = subprocess.run(cmd, cwd=entry['directory'], capture_output=True, text=True)
    return r.returncode, r.stdout, r.stderr


def parse_dump(text, source_dir):
    """{qualified name: (abs file, line)} for record / enum definitions under source_dir"""
    found = {}
    qual_of = {}        # decl address -> qualified name, for "parent 0x.."
    stack = []          # (depth, name, is_function)
    base = []           # qualifier from the "Dumping" header
    cur_file = None

    for line in text.splitlines():
        m = re.match(r'Dumping (.*):$', line)
        if m:
            # the header names the dumped decl itself; its children nest under it
            base = m.group(1).split('::')[:-1]
            stack = []
            continue

        # EVERY line, before anything else: clang's "last file printed"
        # also advances on lines that are not decl nodes (e.g.
        # TemplateArgument, which has no address)
        # ...but not inside a quoted type string: "'.. (lambda at /x.cpp:3:5) ..'"
        # names a file without printing a location, so clang's "last file"
        # does not move
        for f in FILE_RE.findall(QUOTED_RE.sub("''", line)):
            cur_file = f

        n = NODE_RE.match(line)
        if not n:
            continue

        depth = len(n.group(1) or '') // 2
        kind = n.group(2)
        addr = n.group(3)

        while stack and stack[-1][0] >= depth:
            stack.pop()

        in_function = any(s[2] for s in stack)

        if kind in FUNCTION_KINDS:
            stack.append((depth, None, True))
            continue

        name = None

        if kind == 'NamespaceDecl':
            tail = line.rstrip()
            m2 = re.search(r' (\w+)$', tail)
            # an anonymous namespace ends at its location, with no name
            name = ANON if (tail.endswith('>') or not m2) else m2.group(1)
            if m2 and re.search(r'(line|col):\d+:\d+$|col:\d+$', tail):
                name = ANON

        elif kind in ('CXXRecordDecl', 'EnumDecl'):
            if kind == 'CXXRecordDecl':
                md = RECORD_DEF_RE.search(line)
                mn = md or RECORD_ANY_RE.search(line)
            else:
                md = mn = ENUM_RE.search(line) if (' EnumDecl' in line or line.lstrip('|`- ').startswith('EnumDecl')) else None

            name = mn.group(1) if mn else None

            if md and name and not in_function:
                pm = PARENT_RE.search(line)
                if pm and pm.group(1) in qual_of:
                    # out-of-line nested definition: "struct A::B {..}" at
                    # namespace scope -- clang names the real enclosing class
                    qual = qual_of[pm.group(1)] + '::' + name
                else:
                    qual = '::'.join(base + [s[1] for s in stack] + [name])

                lm = NAMELOC_RE.search(line)
                if lm and lm.group(1):
                    line_no = int(lm.group(1))
                else:
                    rs = RANGE_START_RE.search(line)
                    line_no = int(rs.group(1)) if rs else None

                if (cur_file and line_no and ANON not in qual
                        and os.path.normpath(cur_file).startswith(source_dir + os.sep)):
                    found.setdefault(qual, (os.path.normpath(cur_file), line_no))

        if name:
            stack.append((depth, name, False))
            if kind == 'CXXRecordDecl':
                qual_of[addr] = '::'.join(base + [s[1] for s in stack if s[1]])

    return found


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--compile-commands', required=True, help='compile_commands.json')
    ap.add_argument('--source-dir', required=True,
                    help="the subsystem's source directory: its TUs are dumped, its types kept")
    ap.add_argument('--repo-root', required=True, help='paths in the map are relative to this')
    ap.add_argument('--subsystem', help='subsystem name for the map (default: basename of --source-dir)')
    ap.add_argument('--filter', default='xo::', help='qualified-name filter for the dump (default xo::)')
    ap.add_argument('--clang', default='clang++', help='clang driver (default clang++)')
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    ap.add_argument('--output', required=True, help='json file to write')
    args = ap.parse_args(argv)

    source_dir = os.path.normpath(os.path.realpath(args.source_dir))
    repo_root = os.path.normpath(os.path.realpath(args.repo_root))
    subsystem = args.subsystem or os.path.basename(source_dir)

    with open(args.compile_commands) as f:
        db = json.load(f)

    tus = [e for e in db
           if os.path.normpath(os.path.realpath(e['file'])).startswith(source_dir + os.sep)]

    types = {}          # qualified name -> (abs file, line)
    conflicts = []
    failed = []

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        futs = {pool.submit(dump_tu, e, args.clang, args.filter): e for e in tus}
        for fut in concurrent.futures.as_completed(futs):
            e = futs[fut]
            rc, out, err = fut.result()
            if rc != 0:
                failed.append((e['file'], err.strip().splitlines()[-1:] or ['']))
            for q, loc in parse_dump(out, source_dir).items():
                have = types.get(q)
                if have is None:
                    types[q] = loc
                elif have != loc:
                    # a location is a fact about the type: every TU must agree
                    conflicts.append((q, have, loc))

    for f, msg in failed:
        print(f'xo-type-src-map: warning: clang failed on {f}: {msg[0]}', file=sys.stderr)

    # a name defined in two places (e.g. two headers never included together,
    # each defining xo::tree::detail::IteratorBase) cannot be linked
    # unambiguously: reported, left out of 'types', listed under 'conflicts'.
    # Not fatal -- a naming problem in the code should not fail the build
    conflict_locs = {}
    for q, a, b in conflicts:
        conflict_locs.setdefault(q, set()).update([a, b])
        types.pop(q, None)
    for q, locs in sorted(conflict_locs.items()):
        where = ', '.join(f'{os.path.relpath(f, repo_root)}:{l}' for f, l in sorted(locs))
        print(f'xo-type-src-map: warning: {q} defined in more than one place: {where}',
              file=sys.stderr)

    out = {'format': FORMAT,
           'subsystem': subsystem,
           'types': {q: {'file': os.path.relpath(f, repo_root), 'line': l}
                     for q, (f, l) in sorted(types.items())},
           'conflicts': {q: [{'file': os.path.relpath(f, repo_root), 'line': l}
                             for f, l in sorted(locs)]
                         for q, locs in sorted(conflict_locs.items())}}

    tmp = args.output + '.tmp'
    with open(tmp, 'w') as f:
        json.dump(out, f, indent=1)
        f.write('\n')
    os.replace(tmp, args.output)

    print(f'xo-type-src-map: {subsystem}: {len(tus)} TUs, {len(types)} types'
          + (f', {len(conflict_locs)} conflicting' if conflict_locs else '')
          + (f', {len(failed)} TU(s) failed' if failed else ''), file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
