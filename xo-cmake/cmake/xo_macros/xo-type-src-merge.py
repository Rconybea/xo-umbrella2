#!/usr/bin/env python3
"""xo-type-src-merge - combine per-subsystem type -> source maps.

Two jobs:

  --print-closure --edges FILE --root SUBSYSTEM
      print SUBSYSTEM and everything it depends on, directly or
      indirectly, one name per line, per FILE (subsystem-edges, tsort format:
      a line "A B" means B depends on A)

  --output OUT MAP.json ...
      union the maps (each written by xo-type-src-map) into OUT.

  --output OUT --edges FILE --root SUBSYSTEM --map-template T [--root-map M]
      the same, over SUBSYSTEM's closure: each member's map is T with
      "{subsystem}" replaced by its name (e.g. build/{subsystem}/types.json,
      or <prefix>/share/{subsystem}/types.json); SUBSYSTEM's own map is M when
      given (its build directory: it is not installed yet).  The closure is
      computed here, at build time, from the freshest edges available.  A map file
      that does not exist is skipped with a note (its subsystem built without
      XO_ENABLE_SOURCE_MAP, or has no map).  A type in two maps -- never
      legitimate: each type is defined in exactly one subsystem -- is
      reported, and left out.

Interim home: the merge over a subsystem set's dependency closure belongs in
xo-top once xo-cmake is split (.xo-backlog/xo-cmake/issues/07).  See
.xo-backlog/xo-websock/issues/12.
"""

import argparse
import json
import os
import sys

FORMAT = 'xo-type-src-map/1'


def closure(edges_file, root):
    """root and everything it depends on, directly or indirectly (sorted)"""
    deps = {}           # subsystem -> what it depends on

    with open(edges_file) as f:
        for line in f:
            parts = line.split()
            if len(parts) == 2:
                dep, user = parts
                deps.setdefault(user, set()).add(dep)

    seen = set()
    todo = [root]

    while todo:
        x = todo.pop()
        if x in seen:
            continue
        seen.add(x)
        todo.extend(deps.get(x, ()))

    return sorted(seen)


def merge(map_files):
    """(types, conflicts, used, missing)"""
    types = {}          # name -> {file, line}
    owner = {}          # name -> subsystem
    conflicts = {}      # name -> [ {subsystem, file, line}, .. ]
    used, missing = [], []

    for path in map_files:
        if not os.path.exists(path):
            missing.append(path)
            continue

        with open(path) as f:
            m = json.load(f)

        sub = m.get('subsystem', os.path.basename(os.path.dirname(path)))
        used.append(sub)

        for name, loc in m.get('types', {}).items():
            if name in types and types[name] != loc:
                conflicts.setdefault(name, [dict(subsystem=owner[name], **types[name])])
                conflicts[name].append(dict(subsystem=sub, **loc))
            elif name not in types:
                types[name] = loc
                owner[name] = sub

    for name in conflicts:
        types.pop(name, None)

    return types, conflicts, used, missing


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--print-closure', action='store_true')
    ap.add_argument('--edges', help='subsystem-edges (tsort format)')
    ap.add_argument('--root', help='subsystem whose closure to print')
    ap.add_argument('--output', help='merged json to write')
    ap.add_argument('--map-template', help='path of a member\'s map, with {subsystem}')
    ap.add_argument('--root-map', help="the root subsystem's own map")
    ap.add_argument('maps', nargs='*', help='per-subsystem types.json files')
    args = ap.parse_args(argv)

    if args.print_closure:
        if not (args.edges and args.root):
            ap.error('--print-closure needs --edges and --root')
        for name in closure(args.edges, args.root):
            print(name)
        return 0

    if not args.output:
        ap.error('--output required (or --print-closure)')

    maps = list(args.maps)

    if args.map_template:
        if not (args.edges and args.root):
            ap.error('--map-template needs --edges and --root')
        for name in closure(args.edges, args.root):
            if name == args.root and args.root_map:
                maps.append(args.root_map)
            else:
                maps.append(args.map_template.replace('{subsystem}', name))

    types, conflicts, used, missing = merge(maps)

    for path in missing:
        print(f'xo-type-src-merge: note: no map at {path}', file=sys.stderr)
    for name, locs in sorted(conflicts.items()):
        where = ', '.join(f"{l['subsystem']}:{l['file']}:{l['line']}" for l in locs)
        print(f'xo-type-src-merge: warning: {name} in more than one map: {where}',
              file=sys.stderr)

    out = {'format': FORMAT,
           'subsystems': sorted(used),
           'types': dict(sorted(types.items())),
           'conflicts': dict(sorted(conflicts.items()))}

    tmp = args.output + '.tmp'
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(tmp, 'w') as f:
        json.dump(out, f, indent=1)
        f.write('\n')
    os.replace(tmp, args.output)

    print(f'xo-type-src-merge: {len(used)} maps, {len(types)} types'
          + (f', {len(missing)} missing' if missing else '')
          + (f', {len(conflicts)} conflicting' if conflicts else ''), file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
