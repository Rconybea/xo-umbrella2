"""Checks that survive configure_file() substitution."""

import pathlib
import re
import unittest

_HERE = pathlib.Path(__file__).resolve().parent
_XO_LOC_IN = _HERE.parent / "bin" / "xo-loc.in"


class TestConfigureSafety(unittest.TestCase):
    def setUp(self):
        self.text = _XO_LOC_IN.read_text()

    def test_every_at_placeholder_sits_inside_a_string_literal(self):
        # the file must stay importable straight from the source tree, which is
        # how the unit tests load it -- so no bare @VAR@ tokens
        for match in re.finditer(r"@[A-Z0-9_]+@", self.text):
            line = self.text[:match.start()].count("\n") + 1
            source = self.text.splitlines()[line - 1]
            self.assertRegex(
                source, r'"@[A-Z0-9_]+@"',
                "line %d: %r must keep its @VAR@ inside a string literal"
                % (line, source))

    def test_has_a_shebang(self):
        self.assertTrue(self.text.startswith("#!/usr/bin/env python3"))


if __name__ == "__main__":
    unittest.main()
