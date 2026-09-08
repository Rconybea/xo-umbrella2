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


class VisitPoolsTestCase(unittest.TestCase):
    """memory reporting through AllocFlywheel.visit_pools()"""

    def setUp(self):
        self.fw = f.AllocFlywheel.make_default_app(FACET_CX)

    def pools(self, fw=None):
        return (fw or self.fw).visit_pools()

    def test_reports_the_three_pools_in_order(self):
        pools = self.pools()
        self.assertIsInstance(pools, list)
        self.assertEqual([p.name for p in pools], ["store", "strong", "weak"])

    def test_used_grows_with_allocation(self):
        before = self.pools()[0].used
        keep = [o.Float.make(self.fw, float(i)) for i in range(4)]
        self.assertGreater(self.pools()[0].used, before)
        self.assertEqual(len(keep), 4)

    def test_reserved_is_at_least_committed(self):
        for p in self.pools():
            self.assertGreaterEqual(p.reserved, p.committed)
            self.assertGreaterEqual(p.allocated, p.used)

    def test_snapshot_survives_the_visit(self):
        """the info is a snapshot: c++ MemorySizeInfo could not survive here

        resource_name_ is a string_view and detail_ points into the visiting
        frame, so a returned list of them would be a list of dangling reads.
        Materializing is what makes this binding's shape possible at all.
        """
        store = self.pools()[0]
        self.assertEqual(store.name, "store")
        self.assertEqual(store.detail, [])

    def test_detail_histogram_when_the_arena_keeps_headers(self):
        """detail[0] totals; the remaining entries are per type"""
        fw = f.AllocFlywheel.make_app(
            FACET_CX,
            mm.ArenaConfig(name="store", size=1 << 18, store_header_flag=True),
            mm.ArenaConfig(name="strong", size=1 << 12),
            mm.ArenaConfig(name="weak", size=1 << 12))
        keep = [o.Float.make(fw, float(i)) for i in range(5)]

        detail = self.pools(fw)[0].detail
        self.assertEqual(len(detail), 2)
        self.assertEqual(detail[0].tseq, -1)             # totals row
        self.assertEqual(detail[0].n_alloc, len(keep))
        self.assertNotEqual(detail[1].tseq, -1)          # DFloat
        self.assertEqual(detail[1].n_alloc, len(keep))
        self.assertEqual(detail[0].z_alloc, detail[1].z_alloc)


class AppcxVisitPoolsTestCase(unittest.TestCase):
    """FacetAppcx reports its own pools, and only its own"""

    def test_returns_a_list_of_snapshots(self):
        pools = FACET_CX.visit_pools()
        self.assertIsInstance(pools, list)
        self.assertEqual([p.name for p in pools],
                         ["facets-ctl", "facets-slots", "types"])

    def test_does_not_descend_into_indentlog2(self):
        """descending automatically would double-count for a caller walking
        the witness chain -- so the two lists must be disjoint"""
        # indentlog2's pools are per-thread and made on first use, so give
        # this thread something to log before asking (see the next case)
        repr(o.Float.make(f.AllocFlywheel.make_default_app(FACET_CX), 1.0))

        mine = {p.name for p in FACET_CX.visit_pools()}
        theirs = {p.name for p in FACET_CX.indentlog2_appcx().visit_pools()}
        self.assertTrue(mine)
        self.assertTrue(theirs)
        self.assertEqual(mine & theirs, set())

    def test_indentlog2_pools_are_per_thread_and_lazy(self):
        """reporting must not create what it claims to measure

        Indentlog2Appcx uses check_local(), not local(): a thread that has
        never logged owns no scratch arena, and says so.  Needs a fresh
        interpreter -- any earlier case in this process would have made one.
        """
        body = ("import xo_pyfacet as f, xo_pyobject2 as o\n"
                "cx = f.configure_all()\n"
                "il = cx.indentlog2_appcx()\n"
                "print('before', len(il.visit_pools()))\n"
                "repr(o.Float.make(f.AllocFlywheel.make_default_app(cx), 1.0))\n"
                "print('after', len(il.visit_pools()))\n")
        r = subprocess.run([sys.executable, "-c", body],
                           capture_output=True, text=True)
        before, after = r.stdout.split()[1], r.stdout.split()[3]
        self.assertEqual(int(before), 0, r.stderr)
        # how many the sink and scratch arena add is not pinned here: it is
        # structure, not contract (cf. the arena-name note in this module)
        self.assertGreater(int(after), 0)

    def test_capacity_is_reserved_up_front(self):
        for p in FACET_CX.visit_pools():
            self.assertGreater(p.reserved, 0)


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
