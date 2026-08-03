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


if __name__ == "__main__":
    unittest.main()
