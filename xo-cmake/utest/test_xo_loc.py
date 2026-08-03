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


if __name__ == "__main__":
    unittest.main()
