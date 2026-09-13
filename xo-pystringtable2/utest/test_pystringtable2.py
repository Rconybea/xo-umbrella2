"""Unit tests for xo.stringtable2's subsystem-configuration contract.

Run by ctest through the generated xo-python wrapper, which is what puts the
extension modules on PYTHONPATH -- so a failure here is equally a failure of
that wrapper.  See xo_emit_python_wrapper() in xo-cmake.

NB configuration is process-global and one-shot: xo.stringtable2.configure()
throws on a second call.  So every case that needs a DIFFERENT configuration
runs in its own interpreter (see ConfigurationContractTestCase, which spawns
subprocesses); everything else shares the one established in setUpModule.
"""

import subprocess
import sys
import unittest

import xo.indentlog2 as il
import xo.facet as f
import xo.stringtable2 as st

FACET_CX = None
STRINGTABLE2_CX = None


def setUpModule():
    global FACET_CX, STRINGTABLE2_CX
    FACET_CX = f.configure_all()
    STRINGTABLE2_CX = st.configure(st.Stringtable2Config(), FACET_CX)


class ImportTestCase(unittest.TestCase):
    """the wrapper puts every module this suite needs within reach"""

    def test_modules_import(self):
        for mod in (il, f, st):
            self.assertTrue(hasattr(mod, "__file__"), mod)

    def test_expected_types_are_registered(self):
        self.assertTrue(hasattr(st, "Stringtable2Config"))
        self.assertTrue(hasattr(st, "Stringtable2Appcx"))


class ConfigurationTestCase(unittest.TestCase):
    """configure() reports what it actually established"""

    def test_context_type(self):
        self.assertEqual(type(STRINGTABLE2_CX).__name__, "Stringtable2Appcx")

    def test_config_is_readable(self):
        """without this, configuration is write-only from python

        Stringtable2Config carries no settings and is not expected to gain any:
        a StringTable's capacity belongs to whoever constructs one.  So unlike
        the equivalent in xo-pyreflect/utest, this one is not waiting to get
        teeth -- it asserts the accessor exists and that is all there is.
        """
        self.assertEqual(type(STRINGTABLE2_CX.config()).__name__,
                         "Stringtable2Config")

    def test_visit_pools_is_empty_and_stays_empty(self):
        """xo-stringtable2 owns no pool, and this is not a placeholder

        Distinct from ReflectAppcx.visit_pools and PrintJsonAppcx.visit_pools,
        which are empty pending a DArena refactor and should start reporting
        one day.  This one should not: a StringTable has pools and its own
        visit_pools, but it is owned by its constructor's caller.  If this ever
        stops being empty, something has taken ownership it was not meant to.
        """
        self.assertEqual(STRINGTABLE2_CX.visit_pools(), [])


class ConfigurationContractTestCase(unittest.TestCase):
    """cases needing a pristine process -- configuration is one-shot"""

    PREAMBLE = ("import gc, weakref\n"
                "import xo.facet as f, xo.stringtable2 as st\n"
                "fcx = f.configure_all()\n")

    def run_in_fresh_interpreter(self, body):
        return subprocess.run([sys.executable, "-c", self.PREAMBLE + body],
                              capture_output=True, text=True)

    def test_context_is_owned_by_the_caller(self):
        """configure() hands ownership to python; the module keeps nothing"""
        res = self.run_in_fresh_interpreter(
            "print('ST_ACCESSOR', hasattr(st, 'appcx'))\n"
            "cx = st.configure(st.Stringtable2Config(), fcx)\n"
            "print('OWNED', type(cx).__name__)\n")
        self.assertIn("ST_ACCESSOR False", res.stdout)
        self.assertIn("OWNED Stringtable2Appcx", res.stdout)

    def test_second_configure_raises(self):
        """facet registration is process-wide, so a second context could not
        register this subsystem's (facet, impl) pairs differently
        """
        res = self.run_in_fresh_interpreter(
            "st.configure(st.Stringtable2Config(), fcx)\n"
            "try:\n"
            "    st.configure(st.Stringtable2Config(), fcx)\n"
            "    print('NO_RAISE')\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', 'already configured' in str(e))\n")
        self.assertIn("RAISED True", res.stdout)
        self.assertNotIn("NO_RAISE", res.stdout)

    def test_context_keeps_its_dependency_alive(self):
        """keep_alive<0,2>: Stringtable2Appcx holds `const FacetAppcx &', so
        dropping the caller's handle must not collect it

        Observed through a weakref, not by reading the context afterwards:
        pybind holds the patient in its own internals map, which python's gc
        cannot see, so a weakref on the python object is the only observable
        that discriminates.

        Confirmed to discriminate by rebuilding xo_pystringtable2 with the
        keep_alive removed: ALIVE then reports False.
        """
        res = self.run_in_fresh_interpreter(
            "w = weakref.ref(fcx)\n"
            "stcx = st.configure(st.Stringtable2Config(), fcx)\n"
            "del fcx\n"
            "gc.collect()\n"
            "print('ALIVE', w() is not None)\n"
            "del stcx\n"
            "gc.collect()\n"
            "print('RELEASED', w() is None)\n")
        self.assertIn("ALIVE True", res.stdout)
        self.assertIn("RELEASED True", res.stdout)


if __name__ == "__main__":
    unittest.main()
