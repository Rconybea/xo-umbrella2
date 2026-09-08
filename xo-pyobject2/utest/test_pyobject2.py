"""Unit tests for the xo python extension stack, up to xo_pyobject2.

Run by ctest through the generated xo-python wrapper, which is what puts the
extension modules on PYTHONPATH -- so a failure here is equally a failure of
that wrapper.  See xo_emit_python_wrapper() in xo-cmake.

NB configuration is process-global and one-shot: xo_pyfacet.configure_all()
throws on a second call.  So every case that needs a DIFFERENT configuration
runs in its own interpreter (see ConfigurationContractTestCase, which spawns
subprocesses); everything else shares the one established in setUpModule.

Only reproducible facts are asserted.  Not asserted, deliberately: arena names
(anon1, anon2 ... depend on how many sinks the process has made), anything
carrying an address, and ANSI escapes (they depend on the pp config).
"""

import subprocess
import sys
import unittest

import xo_pyarena as mm
import xo_pyindentlog2 as il
import xo_pyfacet as f
import xo_pyobject2 as o

FACET_CX = None


def setUpModule():
    global FACET_CX
    FACET_CX = f.configure_all()


class ImportTestCase(unittest.TestCase):
    """the wrapper puts every module of the stack within reach"""

    def test_modules_import(self):
        for mod in (mm, il, f, o):
            self.assertTrue(hasattr(mod, "__file__"), mod)

    def test_expected_types_are_registered(self):
        self.assertTrue(hasattr(mm, "ArenaConfig"))
        self.assertTrue(hasattr(il, "PrettySink"))
        self.assertTrue(hasattr(f, "AllocFlywheel"))
        self.assertTrue(hasattr(o, "Float"))


class ConfigurationTestCase(unittest.TestCase):
    """configure_all() reports what it actually used"""

    def test_defaults_round_trip(self):
        want_f = f.FacetConfig.make_default()
        want_il = il.Indentlog2Config.make_default()

        got_f = FACET_CX.config()
        got_il = FACET_CX.indentlog2_appcx().config()

        self.assertEqual(got_f.facet_registry_capacity(),
                         want_f.facet_registry_capacity())
        self.assertEqual(got_f.type_registry_capacity(),
                         want_f.type_registry_capacity())
        self.assertEqual(got_il.temp_arena_capacity(),
                         want_il.temp_arena_capacity())

    def test_witness_chain_is_navigable(self):
        """the evidence xo-facet was configured is reachable, not implied"""
        self.assertIsNotNone(FACET_CX.indentlog2_appcx())


class FloatTestCase(unittest.TestCase):
    """boxing a double into an arena, and getting it back out"""

    def setUp(self):
        self.fw = f.AllocFlywheel.make_default_app(FACET_CX)

    def test_value_round_trips(self):
        for v in (0.0, 3.5, -1.25, 1e308):
            self.assertEqual(o.Float.make(self.fw, v).value(), v)

    def test_repr(self):
        self.assertEqual(repr(o.Float.make(self.fw, 3.5)), "3.5")

    def test_pretty_into_a_sink(self):
        sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(80))
        o.Float.make(self.fw, 3.5).pretty(sink)
        self.assertEqual(sink.output(), "3.5")

    def test_a_sink_accumulates(self):
        """pretty() must not complete the record, or composition breaks"""
        sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(80))
        for v in (1.5, 2.5, 3.5):
            o.Float.make(self.fw, v).pretty(sink)
        self.fw.pretty(sink)
        self.assertEqual(sink.output(), "1.52.53.5<AllocFlywheel>")

    def test_handle_keeps_the_flywheel_alive(self):
        """the object outlives every other reference to its arena"""
        import gc
        x = o.Float.make(self.fw, 42.5)
        self.fw = None
        gc.collect()
        self.assertEqual(x.value(), 42.5)


class ConfigurationContractTestCase(unittest.TestCase):
    """cases needing a pristine process -- configuration is one-shot"""

    def run_in_fresh_interpreter(self, body):
        return subprocess.run([sys.executable, "-c", body],
                              capture_output=True, text=True)

    def test_appcx_before_configure_raises(self):
        r = self.run_in_fresh_interpreter(
            "import xo_pyfacet as f\n"
            "try:\n"
            "    f.appcx()\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', e)\n")
        self.assertIn("RAISED", r.stdout)
        self.assertIn("not configured", r.stdout)

    def test_second_configure_raises(self):
        r = self.run_in_fresh_interpreter(
            "import xo_pyfacet as f\n"
            "f.configure_all()\n"
            "try:\n"
            "    f.configure_all()\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', e)\n")
        self.assertIn("RAISED", r.stdout)
        self.assertIn("already configured", r.stdout)

    def test_supplied_config_is_honored(self):
        r = self.run_in_fresh_interpreter(
            "import xo_pyindentlog2 as il, xo_pyfacet as f\n"
            "cx = f.configure_all(f.FacetConfig(4096, 8192),\n"
            "                     il.Indentlog2Config(il.PpConfig.plain(), 32*1024))\n"
            "c = cx.config()\n"
            "print(c.facet_registry_capacity(), c.type_registry_capacity(),\n"
            "      cx.indentlog2_appcx().config().temp_arena_capacity())\n")
        self.assertEqual(r.stdout.split(), ["4096", "8192", "32768"])

    def test_zero_reservation_config_is_rejected(self):
        """a sink that could never accept a byte is refused at construction"""
        r = self.run_in_fresh_interpreter(
            "import xo_pyindentlog2 as il, xo_pyfacet as f\n"
            "f.configure_all()\n"
            "try:\n"
            "    il.PrettySink.make2str(il.PpConfig())\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', e)\n")
        self.assertIn("RAISED", r.stdout)
        self.assertIn("reserves 0 bytes", r.stdout)


if __name__ == "__main__":
    unittest.main()
