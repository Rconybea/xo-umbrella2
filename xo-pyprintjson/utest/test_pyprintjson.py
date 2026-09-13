"""Unit tests for xo.printjson's subsystem-configuration contract.

Run by ctest through the generated xo-python wrapper, which is what puts the
extension modules on PYTHONPATH -- so a failure here is equally a failure of
that wrapper.  See xo_emit_python_wrapper() in xo-cmake.

Scope is deliberately the Appcx contract, not json printing itself: PrintJson's
print() overloads are untested here and elsewhere, which is a separate gap with
a different character.

NB configuration is process-global and one-shot: xo.printjson.configure() throws
on a second call.  So every case that needs a DIFFERENT configuration runs in
its own interpreter (see ConfigurationContractTestCase, which spawns
subprocesses); everything else shares the one established in setUpModule.
"""

import subprocess
import sys
import unittest

import xo.indentlog2 as il
import xo.reflect as r
import xo.printjson as pj

IL_CX = None
REFLECT_CX = None
PRINTJSON_CX = None


def setUpModule():
    global IL_CX, REFLECT_CX, PRINTJSON_CX
    # all three held at module scope: each context declares a keep_alive edge
    # to the one below, so python would keep them anyway -- holding them here
    # makes the stack explicit rather than relying on that.
    IL_CX = il.configure(il.Indentlog2Config.make_default())
    REFLECT_CX = r.configure(r.ReflectConfig(), IL_CX)
    PRINTJSON_CX = pj.configure(pj.PrintJsonConfig(), REFLECT_CX)


class ImportTestCase(unittest.TestCase):
    """the wrapper puts every module this suite needs within reach"""

    def test_modules_import(self):
        for mod in (il, r, pj):
            self.assertTrue(hasattr(mod, "__file__"), mod)

    def test_expected_types_are_registered(self):
        self.assertTrue(hasattr(pj, "PrintJsonConfig"))
        self.assertTrue(hasattr(pj, "PrintJsonAppcx"))
        self.assertTrue(hasattr(pj, "PrintJson"))


class ConfigurationTestCase(unittest.TestCase):
    """configure() reports what it actually established"""

    def test_context_type(self):
        self.assertEqual(type(PRINTJSON_CX).__name__, "PrintJsonAppcx")

    def test_config_is_readable(self):
        """without this, configuration is write-only from python

        PrintJsonConfig carries no settings yet, so this asserts only that the
        accessor exists and returns one.  It gets teeth when PrintJsonConfig
        gains a setting.
        """
        self.assertEqual(type(PRINTJSON_CX.config()).__name__, "PrintJsonConfig")

    def test_printer_table_is_reachable_through_the_context(self):
        """print_json() is the non-singleton route to the printer table

        Same object as PrintJson.instance() today -- the context adopts the
        singleton -- so this asserts reachability, not distinctness.  Its value
        is that callers written this way survive the singleton's retirement
        (.xo-backlog/xo-printjson/issues/03); asserting they differ would fail
        today and asserting they are identical would fail after.
        """
        self.assertIsNotNone(PRINTJSON_CX.print_json())

    def test_visit_pools_is_an_empty_placeholder(self):
        """PrintJsonAppcx::visit_pools reports nothing, deliberately

        xo-printjson has no pools to report until PrintJson uses a DArena.
        Asserted so that the day it starts reporting some, this test fails and
        someone decides what the right answer is -- rather than an empty list
        continuing to read as a measurement.
        """
        self.assertEqual(PRINTJSON_CX.visit_pools(), [])


class ConfigurationContractTestCase(unittest.TestCase):
    """cases needing a pristine process -- configuration is one-shot"""

    PREAMBLE = ("import gc, weakref\n"
                "import xo.indentlog2 as il, xo.reflect as r, xo.printjson as pj\n"
                "ilcx = il.configure(il.Indentlog2Config.make_default())\n"
                "rcx = r.configure(r.ReflectConfig(), ilcx)\n")

    def run_in_fresh_interpreter(self, body):
        return subprocess.run([sys.executable, "-c", self.PREAMBLE + body],
                              capture_output=True, text=True)

    def test_context_is_owned_by_the_caller(self):
        """configure() hands ownership to python; the module keeps nothing

        No appcx() accessor: the caller owns the context, and what is left of
        the one-shot is a flag, not storage -- see test_second_configure_raises.
        """
        res = self.run_in_fresh_interpreter(
            "print('PJ_ACCESSOR', hasattr(pj, 'appcx'))\n"
            "cx = pj.configure(pj.PrintJsonConfig(), rcx)\n"
            "print('OWNED', type(cx).__name__)\n")
        self.assertIn("PJ_ACCESSOR False", res.stdout)
        self.assertIn("OWNED PrintJsonAppcx", res.stdout)

    def test_second_configure_raises(self):
        """the json printer table is process-wide, so a second context could
        not honour a different config -- throwing beats silently ignoring it
        """
        res = self.run_in_fresh_interpreter(
            "pj.configure(pj.PrintJsonConfig(), rcx)\n"
            "try:\n"
            "    pj.configure(pj.PrintJsonConfig(), rcx)\n"
            "    print('NO_RAISE')\n"
            "except RuntimeError as e:\n"
            "    print('RAISED', 'already configured' in str(e))\n")
        self.assertIn("RAISED True", res.stdout)
        self.assertNotIn("NO_RAISE", res.stdout)

    def test_context_keeps_its_dependency_alive(self):
        """keep_alive<0,2>: dropping the caller's handle on the reflect context
        must not collect it while a printjson context stands on it

        PrintJsonAppcx holds `const ReflectAppcx & reflect_appcx_', so without
        this edge python can collect the reflect context while the printjson
        context still refers to it.  Same shape as the xo-pyreflect case, where
        ReflectAppcx holds a `const Indentlog2Appcx &'.

        Worth knowing how recently that became true.  When these bindings were
        written PrintJsonAppcx took its ReflectAppcx and dropped it on the floor
        (`(void)reflect_appcx;'), so removing the keep_alive collected the
        reflect context and print_json() carried on working -- checked, by
        rebuilding without it.  The edge was a tripwire for a hazard that did
        not exist yet.  It exists now: the member landed the same afternoon.

        The dangle is still LATENT rather than observable, because nothing
        dereferences reflect_appcx_ yet.  That is the same shape as
        AllocFlywheel::make_app, which held a `const FacetAppcx &' and was fine
        until python owned the context.  Do not downgrade this test on the
        grounds that nothing reads the member.

        Observed through a weakref, not by reading the context afterwards:
        pybind holds the patient in its own internals map, which python's gc
        cannot see, so a weakref on the python object is the only observable
        that discriminates.
        """
        res = self.run_in_fresh_interpreter(
            "w = weakref.ref(rcx)\n"
            "pjcx = pj.configure(pj.PrintJsonConfig(), rcx)\n"
            "del rcx\n"
            "gc.collect()\n"
            "print('ALIVE', w() is not None)\n"
            "del pjcx\n"
            "gc.collect()\n"
            "print('RELEASED', w() is None)\n")
        self.assertIn("ALIVE True", res.stdout)
        self.assertIn("RELEASED True", res.stdout)


if __name__ == "__main__":
    unittest.main()
