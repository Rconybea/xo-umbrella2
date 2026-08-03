"""Unit tests for xo-loc.

xo-loc lives at xo-cmake/bin/xo-loc.in.  The .in suffix is for CMake's
configure_file(), but the file is deliberately kept valid Python -- the only
substitution is inside a string literal -- so we can import it directly here
without running a configure step.
"""

import importlib.machinery
import importlib.util
import json
import pathlib
import unittest

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
        self.groups = {
            "xo-reader2": {"Code": C(11000, 620, 200), "Build": C(1377, 40, 20)},
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


if __name__ == "__main__":
    unittest.main()
