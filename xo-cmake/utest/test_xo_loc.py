"""Unit tests for xo-loc.

xo-loc lives at xo-cmake/bin/xo-loc.in.  The .in suffix is for CMake's
configure_file(), but the file is deliberately kept valid Python -- the only
substitution is inside a string literal -- so we can import it directly here
without running a configure step.
"""

import csv as csvmod
import importlib.machinery
import importlib.util
import io
import json
import os
import pathlib
import shutil
import tempfile
import unittest
import xml.etree.ElementTree as ET

_HERE = pathlib.Path(__file__).resolve().parent
_XO_LOC = _HERE.parent / "bin" / "xo-loc.in"


def load_xo_loc():
    """Import xo-cmake/bin/xo-loc.in as a module named 'xo_loc'."""
    loader = importlib.machinery.SourceFileLoader("xo_loc", str(_XO_LOC))
    spec = importlib.util.spec_from_loader("xo_loc", loader)
    module = importlib.util.module_from_spec(spec)
    loader.exec_module(module)
    return module


def load_fixture():
    with open(_HERE / "data" / "scc-sample.json") as f:
        return json.load(f)


class TestModuleLoads(unittest.TestCase):
    def test_script_is_valid_python_and_exposes_a_version(self):
        xo_loc = load_xo_loc()
        self.assertIsInstance(xo_loc.XO_LOC_VERSION, str)
        self.assertTrue(xo_loc.XO_LOC_VERSION)


class TestKindOf(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()

    def test_maps_languages_to_kinds(self):
        self.assertEqual(self.xo_loc.kind_of("C++", False), "Code")
        self.assertEqual(self.xo_loc.kind_of("C++ Header", False), "Code")
        self.assertEqual(self.xo_loc.kind_of("CMake", False), "Build")
        self.assertEqual(self.xo_loc.kind_of("Nix", False), "Build")
        self.assertEqual(self.xo_loc.kind_of("Markdown", False), "Docs")
        self.assertEqual(self.xo_loc.kind_of("JSON5", False), "Data")

    def test_unknown_language_falls_back_to_Other(self):
        self.assertEqual(self.xo_loc.kind_of("Brainfuck", False), "Other")

    def test_generated_overrides_the_language_mapping(self):
        # a generated .hpp is still C++, but its kind is Generated -- this is
        # why generated cannot be derived from a language lookup alone
        self.assertEqual(self.xo_loc.kind_of("C++ Header", True), "Generated")
        self.assertEqual(self.xo_loc.kind_of("Brainfuck", True), "Generated")


class TestNormalize(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.rows = self.xo_loc.normalize(load_fixture())

    def test_one_row_per_file(self):
        self.assertEqual(len(self.rows), 8)

    def test_subsystem_is_first_path_component(self):
        by_path = {r.path: r for r in self.rows}
        self.assertEqual(by_path["xo-gc/src/gc/Collector.cpp"].subsystem, "xo-gc")
        self.assertEqual(
            by_path["xo-imgui/include/imgui/imgui.cpp"].subsystem, "xo-imgui")

    def test_carries_loc_and_complexity(self):
        row = next(r for r in self.rows if r.path.endswith("Collector.cpp"))
        self.assertEqual(row.loc, 400)
        self.assertEqual(row.complexity, 44)

    def test_generated_flag_survives_into_the_row(self):
        row = next(r for r in self.rows if r.path.endswith("type/Type.hpp"))
        self.assertTrue(row.generated)
        self.assertEqual(row.kind, "Generated")
        self.assertEqual(row.language, "C++ Header")


class TestNormalizeWalkRoot(unittest.TestCase):
    """REGRESSION: scc echoes its walk root into Location verbatim.

    find_root() hands run_scc an ABSOLUTE root, so Location comes back
    absolute; the naive first-path-component split then yields '' for every
    file, nothing matches known_subsystems, and the tool reports 'nothing to
    draw after filtering'.  Paths must be made relative to the walk root.
    """

    def setUp(self):
        self.xo_loc = load_xo_loc()

    def raw(self, location):
        return [{"Name": "C++", "Files": [
            {"Location": location, "Code": 400, "Complexity": 44,
             "Generated": False}]}]

    def test_absolute_root_prefix_is_stripped(self):
        [row] = self.xo_loc.normalize(
            self.raw("/abs/root/xo-gc/src/gc/Collector.cpp"), root="/abs/root")
        self.assertEqual(row.subsystem, "xo-gc")
        self.assertEqual(row.path, "xo-gc/src/gc/Collector.cpp")

    def test_trailing_slash_on_the_root_is_tolerated(self):
        [row] = self.xo_loc.normalize(
            self.raw("/abs/root/xo-gc/src/gc/Collector.cpp"), root="/abs/root/")
        self.assertEqual(row.subsystem, "xo-gc")

    def test_dot_relative_walk_root_is_stripped(self):
        [row] = self.xo_loc.normalize(
            self.raw("./xo-gc/src/gc/Collector.cpp"), root=".")
        self.assertEqual(row.subsystem, "xo-gc")
        self.assertEqual(row.path, "xo-gc/src/gc/Collector.cpp")

    def test_already_relative_paths_are_left_alone(self):
        [row] = self.xo_loc.normalize(self.raw("xo-gc/src/gc/Collector.cpp"))
        self.assertEqual(row.subsystem, "xo-gc")
        self.assertEqual(row.path, "xo-gc/src/gc/Collector.cpp")


ALL_SUBSYSTEMS = {"xo-gc", "xo-imgui", "xo-type", "xo-cmake", "xo-hashable2"}


class TestSelect(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.rows = self.xo_loc.normalize(load_fixture())

    def sel(self, **kw):
        return self.xo_loc.select(self.rows, ALL_SUBSYSTEMS, **kw)

    def test_default_view_drops_vendored_and_generated(self):
        groups = self.sel().groups
        # imgui.cpp (11375 LOC) is vendored -> xo-imgui keeps nothing here
        self.assertNotIn("xo-imgui", groups)
        # xo-type's only file is generated -> nothing left
        self.assertNotIn("xo-type", groups)
        self.assertIn("xo-gc", groups)

    def test_include_vendored_restores_imgui(self):
        groups = self.sel(include_vendored=True).groups
        self.assertEqual(groups["xo-imgui"]["Code"].loc, 11375)

    def test_include_generated_restores_it_as_its_own_kind(self):
        groups = self.sel(include_generated=True).groups
        self.assertEqual(groups["xo-type"]["Generated"].loc, 120)

    def test_aggregates_by_subsystem_and_kind(self):
        gc = self.sel().groups["xo-gc"]
        self.assertEqual(gc["Code"].loc, 490)        # 400 .cpp + 90 .hpp
        self.assertEqual(gc["Code"].complexity, 49)  # 44 + 5
        self.assertEqual(gc["Code"].nfiles, 2)
        self.assertEqual(gc["Build"].loc, 25)        # CMakeLists.txt
        self.assertEqual(gc["Other"].loc, 7)         # weird.bf

    def test_keeps_the_language_breakdown_for_the_tooltip(self):
        gc = self.sel().groups["xo-gc"]
        self.assertEqual(gc["Code"].languages, {"C++": 400, "C++ Header": 90})

    def test_empty_placeholder_is_reported_not_drawn(self):
        selection = self.sel()
        self.assertNotIn("xo-hashable2", selection.groups)
        self.assertTrue(
            any("xo-hashable2" in n for n in selection.notes),
            "an excluded subsystem must be named in the report")

    def test_min_loc_tests_total_loc_not_code_loc(self):
        # REGRESSION: xo-cmake is 100% Build and has zero Code LOC, but it is a
        # real subsystem.  A 'zero code LOC' emptiness test wrongly drops it.
        groups = self.sel().groups
        self.assertIn("xo-cmake", groups)
        self.assertEqual(groups["xo-cmake"]["Build"].loc, 1066)

    def test_non_xo_paths_are_ignored(self):
        rows = self.rows + [self.xo_loc.Row(
            subsystem="pkgs", path="pkgs/xo-gc.nix", language="Nix",
            kind="Build", generated=False, loc=99, complexity=0)]
        groups = self.xo_loc.select(rows, ALL_SUBSYSTEMS).groups
        self.assertNotIn("pkgs", groups)

    def test_positional_subsystems_restrict_the_output(self):
        groups = self.sel(subsystems=["xo-gc"]).groups
        self.assertEqual(set(groups), {"xo-gc"})

    def test_kinds_filter(self):
        groups = self.sel(kinds=["Build"]).groups
        self.assertEqual(set(groups["xo-gc"]), {"Build"})

    def test_notes_quantify_every_exclusion(self):
        notes = " | ".join(self.sel().notes)
        self.assertIn("11375", notes)  # vendored LOC hidden
        self.assertIn("120", notes)    # generated LOC hidden

    def test_kinds_filter_reports_what_it_hid(self):
        # REGRESSION: --kinds is an exclusion like any other.  It silently
        # dropped 26,338 LOC on the real tree, against the spec's rule that
        # "the tool always prints what it hid".
        selection = self.sel(kinds=["Build"])
        notes = " | ".join(selection.notes)
        self.assertIn("--kinds", notes)
        # 490 xo-gc Code + 7 xo-gc Other; vendored and generated were already
        # excluded upstream and are reported by their own notes
        self.assertIn("497", notes)

    def test_empty_note_respects_positional_subsystems(self):
        # asking for xo-gc should not report placeholders the caller never
        # asked about
        notes = " | ".join(self.sel(subsystems=["xo-gc"]).notes)
        self.assertNotIn("xo-hashable2", notes)
        # ...but it is still named when the whole tree is in view
        self.assertIn("xo-hashable2", " | ".join(self.sel().notes))


class TestSubsystemLoc(unittest.TestCase):
    def test_totals_across_kinds(self):
        xo_loc = load_xo_loc()
        rows = xo_loc.normalize(load_fixture())
        groups = xo_loc.select(rows, ALL_SUBSYSTEMS).groups
        totals = xo_loc.subsystem_loc(groups)
        self.assertEqual(totals["xo-gc"], 522)  # 490 Code + 25 Build + 7 Other


class TestSquarify(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)

    def area(self, rect):
        return rect.w * rect.h

    def test_empty_input(self):
        self.assertEqual(self.xo_loc.squarify([], self.canvas), [])

    def test_single_item_fills_the_rect(self):
        [p] = self.xo_loc.squarify([("a", 10)], self.canvas)
        self.assertEqual(p.key, "a")
        self.assertAlmostEqual(p.rect.w, 1200.0)
        self.assertAlmostEqual(p.rect.h, 800.0)

    def test_tiles_the_rect_exactly(self):
        items = [("s%d" % i, w) for i, w in
                 enumerate([12377, 11011, 7184, 6143, 6075, 5519, 109, 130])]
        placed = self.xo_loc.squarify(items, self.canvas)
        self.assertEqual(len(placed), len(items))
        self.assertAlmostEqual(sum(self.area(p.rect) for p in placed),
                               self.area(self.canvas), places=3)

    def test_area_is_proportional_to_weight(self):
        items = [("a", 3), ("b", 1)]
        placed = {p.key: p.rect for p in self.xo_loc.squarify(items, self.canvas)}
        self.assertAlmostEqual(self.area(placed["a"]) / self.area(placed["b"]),
                               3.0, places=6)

    def test_every_tile_stays_inside_the_canvas(self):
        items = [("s%d" % i, i + 1) for i in range(30)]
        for p in self.xo_loc.squarify(items, self.canvas):
            self.assertGreaterEqual(p.rect.x, -1e-9)
            self.assertGreaterEqual(p.rect.y, -1e-9)
            self.assertLessEqual(p.rect.x + p.rect.w, 1200.0 + 1e-9)
            self.assertLessEqual(p.rect.y + p.rect.h, 800.0 + 1e-9)

    def test_tiles_do_not_overlap(self):
        items = [("s%d" % i, i + 1) for i in range(12)]
        placed = [p.rect for p in self.xo_loc.squarify(items, self.canvas)]
        for i, a in enumerate(placed):
            for b in placed[i + 1:]:
                separated = (a.x + a.w <= b.x + 1e-6 or b.x + b.w <= a.x + 1e-6
                             or a.y + a.h <= b.y + 1e-6 or b.y + b.h <= a.y + 1e-6)
                self.assertTrue(separated, "tiles overlap: %r %r" % (a, b))

    def test_aspect_ratios_are_reasonable(self):
        # the whole point of *squarified* treemaps: no 1000:1 slivers
        items = [("s%d" % i, w) for i, w in
                 enumerate([12377, 11011, 7184, 6143, 6075, 5519, 5002, 4451])]
        for p in self.xo_loc.squarify(items, self.canvas):
            ratio = max(p.rect.w / p.rect.h, p.rect.h / p.rect.w)
            self.assertLess(ratio, 8.0, "sliver: %r" % (p.rect,))

    def test_non_positive_weights_are_skipped(self):
        placed = self.xo_loc.squarify([("a", 10), ("b", 0), ("c", -1)], self.canvas)
        self.assertEqual([p.key for p in placed], ["a"])

    def test_degenerate_rect_yields_nothing(self):
        flat = self.xo_loc.Rect(0.0, 0.0, 0.0, 800.0)
        self.assertEqual(self.xo_loc.squarify([("a", 10)], flat), [])


# The real default-view distribution.  This MUST be the full 62-subsystem table:
# F depends on N as well as T, so a trimmed subset changes the answer.  With all
# 62 entries F = 173.45 and exactly three subsystems are floored; with a
# 14-entry subset F drops to 86 and nothing is floored at all.
REAL_WEIGHTS = {
    "xo-reader2": 12377, "xo-imgui": 11011, "xo-unit": 7184, "xo-ordinaltree": 6143,
    "xo-gc": 6075, "xo-reader": 5519, "xo-arena": 5002, "xo-expression2": 4451,
    "xo-jit": 4387, "xo-alloc": 4359, "xo-alloc2": 4158, "xo-indentlog": 3949,
    "xo-cmake": 3767, "xo-expression": 3268, "xo-indentlog2": 3127, "xo-tokenizer": 3082,
    "xo-interpreter2": 3040, "xo-websock": 2711, "xo-object2": 2563, "xo-ppsink": 2412,
    "xo-kalmanfilter": 2285, "xo-reflect": 2281, "xo-object": 2275, "xo-ratio": 1950,
    "xo-procedure2": 1908, "xo-reactor": 1850, "xo-interpreter": 1796, "xo-facet": 1766,
    "xo-tokenizer2": 1712, "xo-flatstring": 1509, "xo-type": 1377, "xo-stringtable2": 1355,
    "xo-numeric": 1306, "xo-process": 1255, "xo-refcnt": 845, "xo-simulator": 823,
    "xo-distribution": 804, "xo-pykalmanfilter": 801, "xo-printjson": 699,
    "xo-pysimulator": 612, "xo-randomgen": 464, "xo-pyprocess": 458, "xo-pyreactor": 399,
    "xo-timeutil": 381, "xo-callback": 356, "xo-allocutil": 334, "xo-pyjit": 332,
    "xo-subsys": 305, "xo-pyexpression": 283, "xo-testutil": 270, "xo-webutil": 263,
    "xo-pyprintjson": 254, "xo-pyreflect": 244, "xo-reflectutil": 235, "xo-pywebutil": 224,
    "xo-pyunit": 214, "xo-statistics": 199, "xo-pyutil": 196, "xo-pydistribution": 181,
    "xo-pywebsock": 159, "xo-printable2": 130, "xo-symboltable": 109,
}


class TestApplyMinArea(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.canvas_area = 1200.0 * 800.0

    def test_no_floor_needed_leaves_weights_untouched(self):
        weights = {"a": 50000, "b": 50000}
        result = self.xo_loc.apply_min_area(weights, self.canvas_area)
        self.assertEqual(result.weights, weights)
        self.assertEqual(result.floored, [])
        self.assertAlmostEqual(result.inflation, 0.0)

    def test_floors_only_the_small_subsystems(self):
        result = self.xo_loc.apply_min_area(REAL_WEIGHTS, self.canvas_area)
        self.assertEqual(set(result.floored),
                         {"xo-symboltable", "xo-printable2", "xo-pywebsock"})

    def test_distortion_stays_tiny(self):
        result = self.xo_loc.apply_min_area(REAL_WEIGHTS, self.canvas_area)
        self.assertAlmostEqual(result.inflation, 0.000914, places=5)  # ~0.09%

    def test_INVARIANT_every_floored_tile_reaches_the_minimum_area(self):
        # this is the property F was derived to guarantee
        result = self.xo_loc.apply_min_area(REAL_WEIGHTS, self.canvas_area)
        canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        placed = self.xo_loc.squarify(sorted(result.weights.items()), canvas)
        for p in placed:
            self.assertGreaterEqual(p.rect.w * p.rect.h,
                                    self.xo_loc.A_MIN - 1e-6,
                                    "%s is below the legible minimum" % p.key)

    def test_too_many_subsystems_to_floor_is_reported_not_crashed(self):
        # N * a >= 1: the canvas cannot give everyone a legible tile
        weights = {"s%d" % i: 1 for i in range(2000)}
        result = self.xo_loc.apply_min_area(weights, self.canvas_area)
        self.assertEqual(result.weights, weights)
        self.assertEqual(result.floored, [])


class TestLayout(unittest.TestCase):
    # A two-subsystem fixture is too small for the production A_MIN to floor
    # anything, so these pass an explicit min_tile_area chosen to exercise both
    # branches deterministically: at 20000 px^2 the floor is 271 (> 109), so
    # xo-symboltable is floored and lands at ~20601 px^2 -- above one sub-tile
    # but below the 2 * 20000 needed to split it in two.
    MIN_TILE = 20000.0

    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        C = self.xo_loc.Cell
        # Build is 25% of xo-reader2 here: it has to clear MIN_KIND_FRACTION to
        # reach the legibility gate these tests are about.
        self.groups = {
            "xo-reader2": {"Code": C(9000, 620, 200), "Build": C(3000, 40, 20)},
            "xo-symboltable": {"Code": C(76, 3, 4), "Build": C(33, 1, 2)},
        }

    def run_layout(self):
        return self.xo_loc.layout(self.groups, self.canvas,
                                  min_tile_area=self.MIN_TILE)

    def test_big_tile_is_subdivided_by_kind(self):
        tiles, _ = self.run_layout()
        big = next(t for t in tiles if t.subsystem == "xo-reader2")
        self.assertEqual({p.key for p in big.kind_rects}, {"Code", "Build"})

    def test_floored_tile_is_NOT_subdivided(self):
        # it is exactly one label big; splitting it would re-hide it
        tiles, _ = self.run_layout()
        small = next(t for t in tiles if t.subsystem == "xo-symboltable")
        self.assertEqual(small.kind_rects, [])
        self.assertEqual(small.dominant, "Code")

    def test_sub_tiles_tile_their_parent_exactly(self):
        tiles, _ = self.run_layout()
        big = next(t for t in tiles if t.subsystem == "xo-reader2")
        self.assertAlmostEqual(sum(p.rect.w * p.rect.h for p in big.kind_rects),
                               big.rect.w * big.rect.h, places=3)

    def test_reports_the_floor_it_applied(self):
        _, notes = self.run_layout()
        self.assertTrue(any("min-area floor" in n for n in notes))


class TestKindSignificance(unittest.TestCase):
    """A kind earns a sub-tile by being a real FRACTION of its own subsystem.

    An absolute area floor cannot express this: it scales with the subsystem,
    so it keeps a 2% sliver in a big subsystem and drops a 30% share in a
    small one.  The fraction test also bounds tile shape -- a guillotine cut
    gives a kind of fraction f an aspect of 1/(a*f), a >= 1 -- so the floor
    doubles as the no-slivers guarantee.
    """

    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)

    def aspect(self, rect):
        return max(rect.w / rect.h, rect.h / rect.w)

    def test_insignificant_kind_is_not_drawn(self):
        # REGRESSION: xo-imgui is Code 97.9% / Build 2.1%.  Drawn, Build was a
        # 44.5:1 splinter -- and 2% build material is not a fact about the
        # subsystem worth a tile.
        #
        # Nothing is hidden by flattening: the tile still covers the whole
        # subsystem and its tooltip enumerates every kind, so this case needs
        # no fold note.  Only a SUBDIVIDED tile can swallow a kind silently.
        C = self.xo_loc.Cell
        groups = {"xo-imgui": {"Code": C(10778, 600, 150),
                               "Build": C(233, 10, 12, {"CMake": 233})}}
        tiles, _ = self.xo_loc.layout(groups, self.canvas)
        self.assertEqual(tiles[0].kind_rects, [])
        self.assertEqual(tiles[0].dominant, "Code")

    def test_a_fold_inside_a_subdivided_tile_is_reported(self):
        # here the fold IS lossy: Docs gets no tile and no tooltip of its own,
        # and its area is absorbed by Code and Build
        C = self.xo_loc.Cell
        groups = {"xo-b": {"Code": C(4300, 300, 60), "Build": C(700, 20, 10),
                           "Docs": C(90, 0, 3)}}
        tiles, notes = self.xo_loc.layout(groups, self.canvas)
        self.assertEqual({p.key for p in tiles[0].kind_rects}, {"Code", "Build"})
        self.assertTrue(any("folded" in n for n in notes),
                        "a folded kind must be reported, never hidden")

    def test_significant_kind_in_a_small_subsystem_is_kept(self):
        # the point of a FRACTION floor: 30% Build is a real fact about this
        # subsystem even though it is only 300 LOC
        C = self.xo_loc.Cell
        groups = {"xo-small": {"Code": C(700, 40, 9), "Build": C(300, 8, 5)}}
        tiles, _ = self.xo_loc.layout(groups, self.canvas)
        self.assertEqual({p.key for p in tiles[0].kind_rects}, {"Code", "Build"})

    def test_INVARIANT_no_sub_tile_is_illegible_or_a_sliver(self):
        C = self.xo_loc.Cell
        groups = {
            "xo-a": {"Code": C(10778, 600, 150), "Build": C(233, 10, 12)},
            "xo-b": {"Code": C(4300, 300, 60), "Build": C(600, 20, 10),
                     "Docs": C(90, 0, 3)},
            "xo-c": {"Build": C(3100, 90, 40), "Code": C(700, 30, 12)},
            "xo-d": {"Code": C(76, 3, 4), "Build": C(33, 1, 2)},
        }
        tiles, _ = self.xo_loc.layout(groups, self.canvas)
        drawn = 0
        for tile in tiles:
            for placement in tile.kind_rects:
                drawn += 1
                self.assertGreaterEqual(
                    placement.rect.w * placement.rect.h,
                    self.xo_loc.A_MIN - 1e-6,
                    "%s/%s is below the legible minimum"
                    % (tile.subsystem, placement.key))
                self.assertLessEqual(
                    self.aspect(placement.rect), 8.0 + 1e-6,
                    "%s/%s is a sliver" % (tile.subsystem, placement.key))
        self.assertGreater(drawn, 0, "the fixture must exercise subdivision")

    def test_sub_tiles_still_tile_their_parent_exactly(self):
        # folding redistributes the dropped area across the survivors; the
        # parent must still be covered exactly
        C = self.xo_loc.Cell
        groups = {"xo-b": {"Code": C(4300, 300, 60), "Build": C(700, 20, 10),
                           "Docs": C(90, 0, 3)}}
        tiles, _ = self.xo_loc.layout(groups, self.canvas)
        tile = tiles[0]
        self.assertAlmostEqual(
            sum(p.rect.w * p.rect.h for p in tile.kind_rects),
            tile.rect.w * tile.rect.h, places=3)


class TestFindRoot(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.tmp)
        os.makedirs(os.path.join(self.tmp, "xo-cmake"))
        os.makedirs(os.path.join(self.tmp, "xo-gc", "src", "gc"))

    def test_explicit_root_wins(self):
        self.assertEqual(self.xo_loc.find_root(explicit=self.tmp), self.tmp)

    def test_env_is_used_when_no_explicit_root(self):
        self.assertEqual(self.xo_loc.find_root(env=self.tmp), self.tmp)

    def test_searches_upward_from_cwd(self):
        deep = os.path.join(self.tmp, "xo-gc", "src", "gc")
        self.assertEqual(self.xo_loc.find_root(cwd=deep), self.tmp)

    def test_raises_when_no_root_can_be_found(self):
        empty = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, empty)
        with self.assertRaises(self.xo_loc.XoLocError):
            self.xo_loc.find_root(cwd=empty)


class TestKnownSubsystems(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.tmp)
        os.makedirs(os.path.join(self.tmp, "xo-gc"))
        os.makedirs(os.path.join(self.tmp, "xo-cmake"))
        os.makedirs(os.path.join(self.tmp, "pkgs"))
        open(os.path.join(self.tmp, "xo-deps.svg"), "w").close()

    def test_directories_matching_xo_star_only(self):
        found = self.xo_loc.known_subsystems(self.tmp)
        self.assertEqual(found, {"xo-gc", "xo-cmake"})

    def test_excludes_a_FILE_matching_the_glob(self):
        # xo-deps.svg is a real file in this repo root and matches xo-*
        self.assertNotIn("xo-deps.svg", self.xo_loc.known_subsystems(self.tmp))


class TestRunScc(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()

    def test_missing_scc_raises_a_helpful_error(self):
        with self.assertRaises(self.xo_loc.XoLocError) as ctx:
            self.xo_loc.run_scc(".", scc="definitely-not-a-real-binary")
        self.assertIn("scc", str(ctx.exception).lower())

    def test_unrunnable_scc_raises_rather_than_tracebacks(self):
        # REGRESSION: catching only FileNotFoundError let a present-but-not-
        # executable --scc=PATH escape as a PermissionError traceback
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp)
        path = os.path.join(tmp, "not-executable")
        with open(path, "w") as f:
            f.write("#!/bin/sh\necho hi\n")
        os.chmod(path, 0o644)
        with self.assertRaises(self.xo_loc.XoLocError):
            self.xo_loc.run_scc(".", scc=path)


REPO_ROOT = _HERE.parent.parent


@unittest.skipUnless(shutil.which("scc"), "scc not installed")
class TestSccIntegration(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()

    def test_generated_marker_still_matches_the_tree(self):
        # GUARD: the marker is coupled to xo-facet/codegen/*.j2 wording.  If the
        # templates are reflowed this fails loudly rather than silently
        # counting generated code as hand-written.
        raw = self.xo_loc.run_scc(str(REPO_ROOT))
        rows = self.xo_loc.normalize(raw)
        generated = [r for r in rows if r.generated]
        self.assertGreater(
            len(generated), 100,
            "the %r marker matched %d files; has xo-facet/codegen/*.j2 changed?"
            % (self.xo_loc.SCC_GENERATED_MARKER, len(generated)))

    def test_root_walk_excludes_build_artifacts(self):
        # walking a subdirectory instead would re-admit these
        raw = self.xo_loc.run_scc(str(REPO_ROOT))
        rows = self.xo_loc.normalize(raw)
        self.assertEqual([r.path for r in rows if "/.build" in r.path], [])

    def test_locations_resolve_to_real_subsystems(self):
        # REGRESSION: this is the assertion that catches an absolute walk root
        # leaking into Location.  Without it the whole pipeline silently
        # buckets every file under the subsystem '' and draws nothing.
        raw = self.xo_loc.run_scc(str(REPO_ROOT))
        rows = self.xo_loc.normalize(raw, root=str(REPO_ROOT))
        found = {r.subsystem for r in rows}
        self.assertIn("xo-gc", found)
        self.assertIn("xo-cmake", found)
        self.assertNotIn("", found)


class TestParser(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        self.parser = self.xo_loc.build_parser()

    def test_defaults_match_the_spec(self):
        args = self.parser.parse_args([])
        self.assertEqual(args.format, "svg")
        self.assertEqual(args.color, "kind")
        self.assertEqual(args.min_loc, 20)
        self.assertEqual(args.width, 1200)
        self.assertEqual(args.height, 800)
        self.assertFalse(args.include_generated)
        self.assertFalse(args.include_vendored)
        self.assertEqual(args.subsystems, [])

    def test_positional_subsystems(self):
        args = self.parser.parse_args(["xo-gc", "xo-arena"])
        self.assertEqual(args.subsystems, ["xo-gc", "xo-arena"])

    def test_kinds_is_split_on_commas(self):
        args = self.parser.parse_args(["--kinds=Code,Build"])
        self.assertEqual(self.xo_loc.resolve_args(args).kinds, ["Code", "Build"])

    def test_listing_Generated_in_kinds_implies_include_generated(self):
        args = self.xo_loc.resolve_args(
            self.parser.parse_args(["--kinds=Code,Generated"]))
        self.assertTrue(args.include_generated)

    def test_png_is_rejected(self):
        with self.assertRaises(SystemExit):
            self.parser.parse_args(["--format=png"])

    def test_an_unknown_kind_is_named_rather_than_silently_empty(self):
        # REGRESSION: --kinds=code (wrong case) used to yield the unhelpful
        # "nothing to draw after filtering"
        with self.assertRaises(self.xo_loc.XoLocError) as ctx:
            self.xo_loc.resolve_args(self.parser.parse_args(["--kinds=code"]))
        self.assertIn("code", str(ctx.exception))
        self.assertIn("Code", str(ctx.exception))  # names the valid set

    def test_valid_kinds_are_accepted(self):
        args = self.xo_loc.resolve_args(
            self.parser.parse_args(["--kinds=Code,Docs"]))
        self.assertEqual(args.kinds, ["Code", "Docs"])


class TestTableFormats(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        C = self.xo_loc.Cell
        self.groups = {
            "xo-gc": {"Code": C(490, 49, 2), "Build": C(25, 1, 1)},
            "xo-cmake": {"Build": C(1066, 30, 1)},
        }
        canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        self.tiles, _ = self.xo_loc.layout(self.groups, canvas)
        self.args = self.xo_loc.build_parser().parse_args([])

    def test_csv_has_a_row_per_subsystem_kind(self):
        text = self.xo_loc.emit_csv(self.tiles, self.groups, [], self.args)
        rows = list(csvmod.DictReader(io.StringIO(text)))
        self.assertEqual(len(rows), 3)
        gc_code = next(r for r in rows
                       if r["subsystem"] == "xo-gc" and r["kind"] == "Code")
        self.assertEqual(gc_code["loc"], "490")
        self.assertEqual(gc_code["complexity"], "49")
        self.assertEqual(gc_code["nfiles"], "2")

    def test_json_carries_totals_and_notes(self):
        text = self.xo_loc.emit_json(self.tiles, self.groups,
                                     ["excluded 369 generated files"], self.args)
        data = json.loads(text)
        self.assertEqual(data["total_loc"], 1581)
        self.assertEqual(data["subsystems"]["xo-gc"]["loc"], 515)
        self.assertEqual(data["notes"], ["excluded 369 generated files"])

    def test_json_reports_complexity_per_kloc(self):
        text = self.xo_loc.emit_json(self.tiles, self.groups, [], self.args)
        data = json.loads(text)
        # xo-gc: 50 complexity over 515 loc
        self.assertAlmostEqual(data["subsystems"]["xo-gc"]["cx_per_kloc"],
                               1000.0 * 50 / 515, places=3)


class TestMain(unittest.TestCase):
    def test_reports_a_missing_scc_without_a_traceback(self):
        # --root is pinned so this exercises the scc failure wherever it runs
        # from; under ctest the cwd is the build tree, where root discovery
        # would otherwise fail first and mask what this test is about.
        xo_loc = load_xo_loc()
        err = io.StringIO()
        rc = xo_loc.main(["--scc=definitely-not-a-real-binary",
                          "--root=%s" % REPO_ROOT,
                          "--format=csv", "--output=/dev/null"], stderr=err)
        self.assertEqual(rc, 1)
        self.assertIn("scc", err.getvalue().lower())
        self.assertNotIn("Traceback", err.getvalue())

    def test_reports_an_undiscoverable_root_without_a_traceback(self):
        xo_loc = load_xo_loc()
        empty = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, empty)
        cwd = os.getcwd()
        os.chdir(empty)
        self.addCleanup(os.chdir, cwd)
        err = io.StringIO()
        rc = xo_loc.main(["--format=csv", "--output=/dev/null"], stderr=err)
        self.assertEqual(rc, 1)
        self.assertIn("source root", err.getvalue())
        self.assertNotIn("Traceback", err.getvalue())


class TestSvg(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        C = self.xo_loc.Cell
        # Build is 25% of xo-reader2 and 30% of xo-symboltable, so both clear
        # MIN_KIND_FRACTION and subdivide; xo-gc has a single kind.
        self.groups = {
            "xo-reader2": {"Code": C(9000, 620, 200), "Build": C(3000, 40, 20)},
            "xo-gc": {"Code": C(6000, 650, 78)},
            "xo-symboltable": {"Code": C(76, 3, 4), "Build": C(33, 1, 2)},
        }
        canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        self.tiles, self.notes = self.xo_loc.layout(self.groups, canvas)
        self.args = self.xo_loc.build_parser().parse_args([])

    def render(self, **kw):
        for k, v in kw.items():
            setattr(self.args, k, v)
        return self.xo_loc.emit_svg(self.tiles, self.groups, self.notes, self.args)

    def test_output_is_well_formed_xml(self):
        root = ET.fromstring(self.render())
        self.assertTrue(root.tag.endswith("svg"))

    def test_declares_the_canvas_size(self):
        root = ET.fromstring(self.render())
        self.assertEqual(root.get("width"), "1200")
        self.assertEqual(root.get("height"), "800")

    def test_sets_an_explicit_background(self):
        # xo-deps.in:196 records that inheriting the viewer's background
        # renders elements near-invisible
        root = ET.fromstring(self.render())
        rects = root.iter("{http://www.w3.org/2000/svg}rect")
        first = next(rects)
        self.assertEqual(first.get("width"), "1200")
        self.assertEqual(first.get("height"), "800")
        self.assertTrue(first.get("fill"))

    def test_draws_a_rect_per_leaf(self):
        root = ET.fromstring(self.render())
        rects = list(root.iter("{http://www.w3.org/2000/svg}rect"))
        # At the production A_MIN this fixture floors nothing, so:
        #   background                                              -> 1
        #   xo-reader2      Code 75% / Build 25%, 636148 px^2       -> 2
        #   xo-gc           1 kind                    -> flat       -> 1
        #   xo-symboltable  both kinds significant (70/30), but its
        #                   own tile is 404.82 x 14.27 = 28.4:1, so
        #                   no split of it can be squarish -> flat  -> 1
        self.assertEqual(len(rects), 1 + 2 + 1 + 1)

    def test_single_kind_subsystem_is_never_subdivided(self):
        root = ET.fromstring(self.render())
        titles = [t.text for t in root.iter("{http://www.w3.org/2000/svg}title")]
        self.assertEqual([t for t in titles if t.startswith("xo-gc")],
                         [t for t in titles if t.startswith("xo-gc:")])

    def test_labels_the_subsystems(self):
        text = self.render()
        self.assertIn("xo-reader2", text)
        self.assertIn("xo-gc", text)

    def test_tooltip_carries_everything_the_spec_requires(self):
        C = self.xo_loc.Cell
        cell = C(490, 49, 2, {"C++": 400, "C++ Header": 90})
        tip = self.xo_loc._tooltip("xo-gc", "Code", cell)
        self.assertIn("490 LOC", tip)      # loc
        self.assertIn("cx 49", tip)        # complexity
        self.assertIn("100/kLOC", tip)     # cx per kloc
        self.assertIn("2 files", tip)      # file count
        self.assertIn("C++ 400", tip)      # language breakdown
        self.assertIn("C++ Header 90", tip)

    def test_escapes_markup_in_labels(self):
        self.assertEqual(self.xo_loc.svg_escape("a&b<c>"), "a&amp;b&lt;c&gt;")

    def test_kind_mode_colors_by_kind(self):
        self.assertEqual(
            self.xo_loc.tile_color("Build", 0.0, "kind"),
            self.xo_loc.KIND_COLORS["Build"])

    def test_complexity_mode_greys_out_non_code(self):
        self.assertEqual(self.xo_loc.tile_color("Docs", 0.0, "complexity"),
                         self.xo_loc.NEUTRAL_COLOR)
        self.assertNotEqual(self.xo_loc.tile_color("Code", 120.0, "complexity"),
                            self.xo_loc.NEUTRAL_COLOR)

    def test_complexity_scale_is_monotonic(self):
        low = self.xo_loc.tile_color("Code", 20.0, "complexity")
        high = self.xo_loc.tile_color("Code", 160.0, "complexity")
        self.assertNotEqual(low, high)

    def test_every_kind_has_a_color(self):
        # a kind with no slot would fall back to Other and silently lie
        for kind in ["Code", "Build", "Docs", "Data", "Generated", "Other"]:
            self.assertIn(kind, self.xo_loc.KIND_COLORS)

    def test_kind_colors_are_distinct(self):
        # two kinds sharing a hex would make the color dimension meaningless
        hexes = list(self.xo_loc.KIND_COLORS.values())
        self.assertEqual(len(hexes), len(set(hexes)))


class TestLabelContrast(unittest.TestCase):
    """Labels are judged against the tile they sit on, never the canvas.

    A fixed label colour cannot work across this palette: white on Generated
    is 2.35:1 and white on the pale end of the complexity ramp is 1.54:1,
    while ink on Build is 2.41:1.
    """

    WCAG_LARGE_TEXT = 3.0

    def setUp(self):
        self.xo_loc = load_xo_loc()

    def test_contrast_ratio_matches_wcag(self):
        # white on black is the definitional 21:1; a colour against itself 1:1
        self.assertAlmostEqual(
            self.xo_loc.contrast_ratio("#ffffff", "#000000"), 21.0, places=2)
        self.assertAlmostEqual(
            self.xo_loc.contrast_ratio("#2a78d6", "#2a78d6"), 1.0, places=6)

    def test_every_kind_fill_gets_a_readable_label(self):
        for kind, fill in self.xo_loc.KIND_COLORS.items():
            chosen, ratio = self.xo_loc.label_color(fill)
            self.assertGreaterEqual(
                ratio, self.WCAG_LARGE_TEXT,
                "%s label on %s is only %.2f:1" % (kind, fill, ratio))

    def test_every_complexity_shade_gets_a_readable_label(self):
        for shade in list(self.xo_loc.CX_RAMP) + [self.xo_loc.NEUTRAL_COLOR]:
            chosen, ratio = self.xo_loc.label_color(shade)
            self.assertGreaterEqual(
                ratio, self.WCAG_LARGE_TEXT,
                "label on %s is only %.2f:1" % (shade, ratio))

    def test_pale_fills_take_ink_and_dark_fills_take_white(self):
        self.assertEqual(self.xo_loc.label_color("#b49ee3")[0],
                         self.xo_loc.LABEL_INK)
        self.assertEqual(self.xo_loc.label_color("#843826")[0],
                         self.xo_loc.LABEL_ON_DARK)

    def test_label_is_judged_on_the_worst_fill_it_covers(self):
        # ink is fine on the pale Generated fill (8.38:1) but poor on the rust
        # Build fill (2.41:1); spanning both, neither colour may be assumed
        ink, worst = self.xo_loc.label_color(
            self.xo_loc.KIND_COLORS["Generated"], self.xo_loc.KIND_COLORS["Build"])
        self.assertAlmostEqual(
            worst,
            min(self.xo_loc.contrast_ratio(ink, self.xo_loc.KIND_COLORS["Generated"]),
                self.xo_loc.contrast_ratio(ink, self.xo_loc.KIND_COLORS["Build"])),
            places=6)

    def test_INVARIANT_every_drawn_label_clears_the_minimum_on_every_fill(self):
        # REGRESSION: the label box is 64 px wide but is gated on the PARENT
        # tile, so on a narrow first sub-tile it can straddle two fills.  It
        # must be readable on all of them, or not drawn at all.
        C = self.xo_loc.Cell
        groups = {
            "xo-a": {"Code": C(9000, 600, 150), "Build": C(3000, 90, 40)},
            "xo-b": {"Generated": C(2000, 10, 30), "Build": C(1800, 60, 25)},
            "xo-c": {"Code": C(700, 40, 9), "Docs": C(300, 0, 5)},
        }
        canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        args = self.xo_loc.build_parser().parse_args(["--include-generated"])
        tiles, notes = self.xo_loc.layout(groups, canvas)
        svg = self.xo_loc.emit_svg(tiles, groups, notes, args)

        root = ET.fromstring(svg)
        NS = "{http://www.w3.org/2000/svg}"
        rects = [r for r in root.iter(NS + "rect")][1:]      # drop background
        boxes = [(self.xo_loc.Rect(*(float(r.get(k))
                                     for k in ("x", "y", "width", "height"))),
                  r.get("fill")) for r in rects]

        drawn = 0
        for text in root.iter(NS + "text"):
            drawn += 1
            x, y = float(text.get("x")), float(text.get("y"))
            box = self.xo_loc.Rect(x, y - 11, self.xo_loc.LABEL_W,
                                   self.xo_loc.LABEL_H)
            touched = [f for rect, f in boxes if self.xo_loc._overlaps(box, rect)]
            for fill in touched:
                ratio = self.xo_loc.contrast_ratio(text.get("fill"), fill)
                self.assertGreaterEqual(
                    ratio, self.WCAG_LARGE_TEXT,
                    "label %r sits on %s at only %.2f:1"
                    % (text.text, fill, ratio))
        self.assertGreater(drawn, 0, "the fixture must draw some labels")


class TestHtml(unittest.TestCase):
    def setUp(self):
        self.xo_loc = load_xo_loc()
        C = self.xo_loc.Cell
        self.groups = {
            "xo-reader2": {"Code": C(9000, 620, 200), "Build": C(3000, 40, 20)},
            "xo-gc": {"Code": C(6000, 650, 78)},
        }
        canvas = self.xo_loc.Rect(0.0, 0.0, 1200.0, 800.0)
        self.tiles, self.notes = self.xo_loc.layout(self.groups, canvas)
        self.args = self.xo_loc.build_parser().parse_args([])
        self.html = self.xo_loc.emit_html(self.tiles, self.groups,
                                          self.notes, self.args)

    def test_is_a_complete_document(self):
        self.assertTrue(self.html.lstrip().startswith("<!doctype html>"))
        self.assertIn("</html>", self.html)

    def test_embeds_the_svg(self):
        self.assertIn("<svg", self.html)
        self.assertIn("xo-reader2", self.html)

    def test_is_self_contained(self):
        # no CDN, no external stylesheet, no remote font
        for forbidden in ("http://", "https://cdn", "<link", "src=\"http"):
            self.assertNotIn(forbidden, self.html.replace(
                "http://www.w3.org/2000/svg", ""))

    def test_carries_the_exclusion_report_visibly(self):
        notes = ["excluded 369 generated files (10968 LOC)"]
        html = self.xo_loc.emit_html(self.tiles, self.groups, notes, self.args)
        # the report must be visible in the page, not just an svg comment
        self.assertIn("excluded 369 generated files", html)
        self.assertIn('class="notes"', html)

    def test_ships_a_legend(self):
        self.assertIn('class="legend"', self.html)
        self.assertIn("Build", self.html)

    def test_handles_both_color_schemes(self):
        self.assertIn("prefers-color-scheme", self.html)

    def test_has_the_interaction_hooks(self):
        self.assertIn("addEventListener", self.html)

    def test_complexity_mode_legend_describes_the_ramp(self):
        args = self.xo_loc.build_parser().parse_args(["--color=complexity"])
        html = self.xo_loc.emit_html(self.tiles, self.groups, [], args)
        self.assertIn("cx", html)
        self.assertIn(self.xo_loc.NEUTRAL_COLOR, html)

    def test_escapes_markup_coming_from_notes(self):
        html = self.xo_loc.emit_html(self.tiles, self.groups,
                                     ["<script>alert(1)</script>"], self.args)
        self.assertNotIn("<script>alert(1)</script>", html)
        self.assertIn("&lt;script&gt;", html)


if __name__ == "__main__":
    unittest.main()
