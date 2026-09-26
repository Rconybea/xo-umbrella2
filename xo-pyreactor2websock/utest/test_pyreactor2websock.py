"""Smoke test for xo.reactor2websock.

Nothing at the xo.reactor level is constructible from python, so these cases
cannot push a real source through an endpoint -- the C++ utest
(xo-reactor2websock/utest) does that.  What they pin is the part only python
can see: the module initializes (which imports xo.reactor and xo.webutil), and
both endpoint builders are present and type-check their arguments against
xo.reactor's classes.

See .xo-backlog/xo-websock/issues/02.
"""

import unittest


class ImportTestCase(unittest.TestCase):
    def test_module_imports(self):
        import xo.reactor2websock as r2w
        self.assertTrue(hasattr(r2w, "stream_endpoint_descr"))
        self.assertTrue(hasattr(r2w, "http_endpoint_descr"))

    def test_builders_left_xo_reactor(self):
        """the member versions are gone from xo.reactor -- one home, not two"""
        import xo.reactor as reactor
        self.assertFalse(hasattr(reactor.AbstractSource, "stream_endpoint_descr"))
        self.assertFalse(hasattr(reactor.AbstractEventStore, "http_endpoint_descr"))


class TypeCheckTestCase(unittest.TestCase):
    def test_stream_endpoint_rejects_a_non_source(self):
        import xo.reactor2websock as r2w
        with self.assertRaises(TypeError):
            r2w.stream_endpoint_descr("not a source", "/ws/x")

    def test_http_endpoint_rejects_a_non_store(self):
        import xo.reactor2websock as r2w
        with self.assertRaises(TypeError):
            r2w.http_endpoint_descr(42, "/es")


if __name__ == "__main__":
    unittest.main()
