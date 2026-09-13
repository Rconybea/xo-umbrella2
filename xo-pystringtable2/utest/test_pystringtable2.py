"""Unit tests for xo.stringtable2: subsystem configuration, and String.

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
        self.assertTrue(hasattr(st, "String"))

    def test_imported_types_are_reachable(self):
        """String.make and String.pretty name types this module does not own

        AllocFlywheel comes from xo.facet, PpSink from xo.indentlog2, and
        pybind permits one registration per c++ type -- so pystringtable2
        imports both at init rather than registering them.  Without those
        imports the signatures below would not resolve.
        """
        self.assertTrue(hasattr(f, "AllocFlywheel"))
        self.assertTrue(hasattr(il, "PrettySink"))


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



class StringTestCase(unittest.TestCase):
    """copying a string into an arena, and getting it back out

    Mirrors FloatTestCase in xo-pyobject2/utest, which is the reference for how
    a representation is bound (.xo-backlog/pyobject2/spec.md).
    """

    def setUp(self):
        self.fw = f.AllocFlywheel.make_default_app(FACET_CX)

    def test_value_round_trips(self):
        for v in ("", "a", "hello", "x" * 200):
            self.assertEqual(st.String.make(self.fw, v).value(), v)

    def test_str_agrees_with_value(self):
        s = st.String.make(self.fw, "hello")
        self.assertEqual(str(s), s.value())

    def test_repr(self):
        """unquoted, like Float's -- __repr__ is the pretty rendering

        Worth pinning because it is the surprising half: repr() of a python str
        would quote.  This one goes through TempPpSink, so it reports what the
        object prints as, not what it would be typed as.
        """
        self.assertEqual(repr(st.String.make(self.fw, "hello")), "hello")

    def test_pretty_into_a_sink(self):
        sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(80))
        st.String.make(self.fw, "hello").pretty(sink)
        self.assertEqual(sink.output(), "hello")

    def test_a_sink_accumulates(self):
        """pretty() must not complete the record, or composition breaks"""
        sink = il.PrettySink.make2str(il.PpConfig.scratch_plain(80))
        for v in ("ab", "cd", "ef"):
            st.String.make(self.fw, v).pretty(sink)
        self.fw.pretty(sink)
        self.assertEqual(sink.output(), "abcdef<AllocFlywheel>")

    def test_capacity_is_len_plus_nul(self):
        """DString reserves size+1 chars: the null terminator is real storage

        _from_view_aux sets capacity_ = len + 1 (DString.cpp).  So for anything
        make() produces this accessor is derivable, and it only becomes
        interesting against DString::empty(mm, cap), which python cannot reach.
        """
        for v in ("", "a", "hello"):
            self.assertEqual(st.String.make(self.fw, v).capacity(), len(v) + 1)

    def test_len_is_bytes_not_characters(self):
        """the one place this wrapper can mislead

        __len__ reports DString's byte extent; len(s.value()) counts python
        codepoints.  They agree for ascii and diverge for anything else, so a
        case with both is the only way to keep the distinction honest.
        """
        ascii_s = st.String.make(self.fw, "hello")
        self.assertEqual(len(ascii_s), len(ascii_s.value()))

        multi = st.String.make(self.fw, "\u00e9\u4e2d")
        self.assertEqual(len(multi.value()), 2)    # codepoints
        self.assertEqual(len(multi), 5)            # utf-8 bytes

    def test_embedded_nul_survives(self):
        """size_ is the authority on extent, not the null terminator

        The python half of what xo-stringtable2/utest/json_render.test.cpp pins
        for the json printer: an accessor reading through
        operator std::string_view() would truncate to "a" here.
        """
        s = st.String.make(self.fw, "a\x00b")
        self.assertEqual(s.value(), "a\x00b")
        self.assertEqual(len(s), 3)

    def test_handle_keeps_the_flywheel_alive(self):
        """the object outlives every other reference to its arena"""
        import gc
        x = st.String.make(self.fw, "survives")
        self.fw = None
        gc.collect()
        self.assertEqual(x.value(), "survives")


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
