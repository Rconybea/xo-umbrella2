"""Unit tests for xo.reflect's subsystem-configuration contract.

Run by ctest through the generated xo-python wrapper, which is what puts the
extension modules on PYTHONPATH -- so a failure here is equally a failure of
that wrapper.  See xo_emit_python_wrapper() in xo-cmake.

Scope is deliberately the Appcx contract, not reflection itself: TypeDescr,
Metatype and SelfTagging are untested here and elsewhere, which is a separate
gap with a different character.

NB configuration is process-global and one-shot: xo.reflect.configure() throws
on a second call.  So every case that needs a DIFFERENT configuration runs in
its own interpreter (see ConfigurationContractTestCase, which spawns
subprocesses); everything else shares the one established in setUpModule.
"""

import gc
import subprocess
import sys
import unittest
import weakref

import xo.indentlog2 as il
import xo.reflect as r

IL_CX = None
REFLECT_CX = None


def setUpModule():
    global IL_CX, REFLECT_CX
    # held at module scope: REFLECT_CX holds a c++ reference into IL_CX, and
    # the keep_alive edge is what makes that safe -- see
    # ConfigurationContractTestCase.test_context_keeps_its_dependency_alive
    IL_CX = il.configure(il.Indentlog2Config.make_default())
    REFLECT_CX = r.configure(r.ReflectConfig(), IL_CX)


class ImportTestCase(unittest.TestCase):
    """the wrapper puts every module this suite needs within reach"""

    def test_modules_import(self):
        for mod in (il, r):
            self.assertTrue(hasattr(mod, "__file__"), mod)

    def test_expected_types_are_registered(self):
        self.assertTrue(hasattr(r, "ReflectConfig"))
        self.assertTrue(hasattr(r, "ReflectAppcx"))
        self.assertTrue(hasattr(r, "TypeDescr"))


class ConfigurationTestCase(unittest.TestCase):
    """configure() reports what it actually established"""

    def test_context_type(self):
        self.assertEqual(type(REFLECT_CX).__name__, "ReflectAppcx")

    def test_config_is_readable(self):
        """without this, configuration is write-only from python

        ReflectConfig carries no settings yet, so this asserts only that the
        accessor exists and returns one.  It gets teeth when ReflectConfig
        gains a capacity.
        """
        self.assertEqual(type(REFLECT_CX.config()).__name__, "ReflectConfig")

    def test_visit_pools_is_an_empty_placeholder(self):
        """ReflectAppcx::visit_pools reports nothing, deliberately

        xo-reflect has no pools to report until TypeDescrTable is represented
        with a DArena.  Asserted so that the day it starts reporting some, this
        test fails and someone decides what the right answer is -- rather than
        an empty list continuing to read as a measurement.
        """
        self.assertEqual(REFLECT_CX.visit_pools(), [])


class ConfigurationContractTestCase(unittest.TestCase):
    """cases needing a pristine process -- configuration is one-shot"""

    def run_in_fresh_interpreter(self, body):
        return subprocess.run([sys.executable, "-c", body],
                              capture_output=True, text=True)

    def test_context_is_owned_by_the_caller(self):
        """configure() hands ownership to python; the module keeps nothing

        No appcx() accessor: the caller owns the context, and what is left of
        the one-shot is a flag, not storage -- see test_second_configure_raises.
        """
        res = self.run_in_fresh_interpreter(
            "import xo.indentlog2 as il, xo.reflect as r\n"
            "print('R_ACCESSOR', hasattr(r, 'appcx'))\n"
            "cx = r.configure(r.ReflectConfig(),"
            "                 il.configure(il.Indentlog2Config.make_default()))\n"
            "print('OWNED', type(cx).__name__)\n")
        self.assertIn("R_ACCESSOR False", res.stdout)
        self.assertIn("OWNED ReflectAppcx", res.stdout)

    def test_second_configure_raises(self):
        """TypeDescrTable is process-wide, so a second context could not honour
        a different config -- throwing beats silently ignoring it
        """
        res = self.run_in_fresh_interpreter(
            "import xo.indentlog2 as il, xo.reflect as r\n"
            "ilcx = il.configure(il.Indentlog2Config.make_default())\n"
            "r.configure(r.ReflectConfig(), ilcx)\n"
            "try:\n"
            "    r.configure(r.ReflectConfig(), ilcx)\n"
            "    print('NO_RAISE')\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', 'already configured' in str(e))\n")
        self.assertIn("RAISED True", res.stdout)
        self.assertNotIn("NO_RAISE", res.stdout)

    def test_context_keeps_its_dependency_alive(self):
        """keep_alive<0,2>: the reflect context holds a c++ reference into the
        indentlog2 one, so dropping the caller's handle must not collect it

        Observed through a weakref, NOT by reading the context afterwards:
        without the keep_alive the c++ object is freed, and reading it is a
        use-after-free that happens to return the right bytes.  pybind holds
        the patient in its own internals map, which python's gc cannot see
        (gc.get_referents is empty), so a weakref on the python object is the
        observable that discriminates.

        Confirmed to discriminate by rebuilding xo_pyreflect with the
        keep_alive removed: ALIVE then reports False.
        """
        res = self.run_in_fresh_interpreter(
            "import gc, weakref, xo.indentlog2 as il, xo.reflect as r\n"
            "ilcx = il.configure(il.Indentlog2Config.make_default())\n"
            "w = weakref.ref(ilcx)\n"
            "rcx = r.configure(r.ReflectConfig(), ilcx)\n"
            "del ilcx\n"
            "gc.collect()\n"
            "print('ALIVE', w() is not None)\n"
            "del rcx\n"
            "gc.collect()\n"
            "print('RELEASED', w() is None)\n")
        self.assertIn("ALIVE True", res.stdout)
        self.assertIn("RELEASED True", res.stdout)


if __name__ == "__main__":
    unittest.main()
