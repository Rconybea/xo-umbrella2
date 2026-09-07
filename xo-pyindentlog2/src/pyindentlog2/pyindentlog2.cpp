/* @file pyindentlog2.cpp
 *
 * python bindings for xo-indentlog2 (the pretty-printing / logging stack).
 *
 * SCOPE.  PrettySink is bound as an OPAQUE HANDLE.  Python cannot drive it --
 * there is no put()/begin()/split()/end(), and no pp().  It exists so python
 * can construct a sink, own it, and hand it to some other xo python binding
 * that wants somewhere to render into.  Rendering is always initiated on the
 * c++ side, because sink.pp(x) resolves Prettifier<T> at compile time and a
 * python object has no Prettifier.
 *
 * Consumer modules should take `PpSink &` rather than `PrettySink &`: PpSink
 * is registered below as an abstract base, so a consumer written against the
 * protocol accepts any sink we later bind (a FlatSink, say) without change.
 *
 * LIFETIME.  py::init factories hand pybind11 ownership, so a sink lives
 * exactly as long as python holds a reference and its c++ destructor runs when
 * the last one drops.  Sinks are independent: each owns its own PpState,
 * token streambuf and LogBuffer, so several live sinks do not interfere
 * (verified by construction+interleaved-write probe, 2026-09-06).
 */

#include "pyindentlog2.hpp"
#include <xo/pyarena/pyarena.hpp>
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/indentlog2/print/PpConfig.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <string>

namespace xo {
    namespace py = pybind11;

    using xo::mm::ArenaConfig;

    namespace pp {
        PYBIND11_MODULE(PYINDENTLOG2_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.indentlog2";

            /* PpLogbufConfig's ctor and PpConfig::with_logbuf() name
             * ArenaConfig, which xo_pyarena registers.  pybind11 permits
             * exactly one registration per c++ type, so we import rather than
             * re-register: without this the signatures do not resolve.
             */
            PYARENA_IMPORT_MODULE();


            // ----------------------------------------------------------------
            // PpStyle -- coloring, shared with FlatSink.
            // Opaque: the color_spec members would need their own bindings,
            // and the two factories cover what a caller chooses between.

            py::class_<PpStyle>(m, "PpStyle")
                .def(py::init<>())
                .def_static("plain", &PpStyle::plain,
                            "no color at all -- for pinning rendered text")
                .def_static("colored", &PpStyle::colored,
                            "hardwired default colors")
                .def_readwrite("color_enabled", &PpStyle::color_enabled)
                .def_readwrite("tag_value_offset", &PpStyle::tag_value_offset)
                .def("__repr__",
                     [](const PpStyle & x) {
                         return std::string("<PpStyle color_enabled=")
                             + (x.color_enabled ? "True" : "False") + ">";
                     });

            // ----------------------------------------------------------------
            // PpLayoutConfig -- margins and nesting.

            py::class_<PpLayoutConfig>(m, "PpLayoutConfig")
                .def(py::init<>())
                .def(py::init<std::uint32_t, std::uint32_t,
                              std::uint32_t, std::uint32_t>(),
                     py::arg("indent_width"),
                     py::arg("soft_right_margin"),
                     py::arg("hard_right_margin"),
                     py::arg("hard_max_nesting"))

                .def_property_readonly("indent_width", &PpLayoutConfig::indent_width)
                .def_property_readonly("soft_right_margin", &PpLayoutConfig::soft_right_margin)
                .def_property_readonly("hard_right_margin", &PpLayoutConfig::hard_right_margin)
                .def_property_readonly("hard_max_nesting", &PpLayoutConfig::hard_max_nesting)

                /* fluent setters COPY, matching c++ -- they return a new
                 * config rather than mutating self
                 */
                .def("with_indent_width", &PpLayoutConfig::with_indent_width, py::arg("x"))
                .def("with_soft_right_margin", &PpLayoutConfig::with_soft_right_margin, py::arg("x"))
                .def("with_hard_right_margin", &PpLayoutConfig::with_hard_right_margin, py::arg("x"))
                .def("with_hard_max_nesting", &PpLayoutConfig::with_hard_max_nesting, py::arg("x"))

                .def("__repr__",
                     [](const PpLayoutConfig & x) {
                         return ("<PpLayoutConfig indent_width="
                                 + std::to_string(x.indent_width())
                                 + " soft_right_margin=" + std::to_string(x.soft_right_margin())
                                 + " hard_right_margin=" + std::to_string(x.hard_right_margin())
                                 + ">");
                     });

            // ----------------------------------------------------------------
            // PpLogbufConfig -- the arena the sink renders into.
            //
            py::class_<PpLogbufConfig>(m, "PpLogbufConfig")
                .def(py::init<>())
                .def(py::init<const ArenaConfig &, bool>(),
                     py::arg("logbuf_config"), py::arg("debug_flag") = false)
                .def_property_readonly("logbuf_config",
                                       &PpLogbufConfig::logbuf_config)
                .def_property_readonly("logbuf_debug_flag",
                                       &PpLogbufConfig::logbuf_debug_flag)
                .def("with_logbuf_config",
                     &PpLogbufConfig::with_logbuf_config, py::arg("x"))
                .def("with_logbuf_debug_flag",
                     &PpLogbufConfig::with_logbuf_debug_flag, py::arg("x"))
                .def("with_name", &PpLogbufConfig::with_name, py::arg("name"))
                .def("__repr__",
                     [](const PpLogbufConfig & x) {
                         return std::string("<PpLogbufConfig debug=")
                             + (x.logbuf_debug_flag() ? "True" : "False") + ">";
                     });

            // ----------------------------------------------------------------
            // PpConfig

            py::class_<PpConfig>(m, "PpConfig")
                .def(py::init<>())
                .def(py::init<const PpLayoutConfig &, const PpLogbufConfig &,
                              const PpStyle &>(),
                     py::arg("layout"), py::arg("logbuf"), py::arg("style"))

                .def_static("plain", &PpConfig::plain,
                            "plain (uncolored) + anonymous arena")
                .def_static("colored", &PpConfig::colored,
                            "colored + anonymous arena")
                .def_static("scratch_plain", &PpConfig::scratch_plain,
                            py::arg("margin"),
                            "temporary plain config at the given soft right margin")
                .def_static("scratch_colored", &PpConfig::scratch_colored,
                            py::arg("margin"))
                .def_static("scratch_aux", &PpConfig::scratch_aux,
                            py::arg("basename"), py::arg("margin"), py::arg("style"))

                .def_property_readonly("layout", &PpConfig::layout)
                .def_property_readonly("logbuf", &PpConfig::logbuf)
                .def_property_readonly("style", &PpConfig::style)

                /* fluent setters COPY, matching c++ */
                .def("with_layout", &PpConfig::with_layout, py::arg("x"))
                .def("with_logbuf", &PpConfig::with_logbuf, py::arg("x"))
                .def("with_style", &PpConfig::with_style, py::arg("x"))

                .def("with_indent_width", &PpConfig::with_indent_width, py::arg("x"))
                .def("with_soft_right_margin", &PpConfig::with_soft_right_margin, py::arg("x"))
                .def("with_hard_right_margin", &PpConfig::with_hard_right_margin, py::arg("x"))
                .def("with_hard_max_nesting", &PpConfig::with_hard_max_nesting, py::arg("x"))

                .def("with_logbuf_config", &PpConfig::with_logbuf_config, py::arg("x"))
                .def("with_logbuf_debug_flag", &PpConfig::with_logbuf_debug_flag, py::arg("x"))
                .def("with_logbuf_size", &PpConfig::with_logbuf_size, py::arg("z"))
                .def("with_logbuf_name", &PpConfig::with_logbuf_name, py::arg("name"))

                .def("__repr__",
                     [](const PpConfig & x) {
                         return ("<PpConfig soft_right_margin="
                                 + std::to_string(x.layout().soft_right_margin())
                                 + " indent_width="
                                 + std::to_string(x.layout().indent_width())
                                 + " color="
                                 + (x.style().color_enabled ? "True" : "False")
                                 + ">");
                     });

            // ----------------------------------------------------------------
            // PpSink -- abstract base, so consumer bindings can be written
            // against the protocol rather than against PrettySink.
            // Not constructible from python.

            py::class_<PpSink>(m, "PpSink");

            // ----------------------------------------------------------------
            // PrettySink -- opaque handle; see SCOPE at the top of this file.

            py::class_<PrettySink, PpSink>(m, "PrettySink")
                /* pybind11 MOVES a factory's return value onto the heap.
                 * That is safe: PrettySink has a hand-written move ctor which
                 * repairs its two interior pointers (see PrettySink.hpp).  The
                 * compiler-generated one was not -- a moved sink rendered
                 * empty or crashed, and this binding is what first exposed it.
                 */
                .def_static("make2str", &PrettySink::make2str, py::arg("cfg"),
                            "sink that accumulates; read it back with output()")
                .def_static("make2cout", &PrettySink::make2cout, py::arg("cfg"),
                            "sink that drains to stdout on complete()")

                /* NB returns a COPY.  PrettySink::output() hands back a
                 * string_view valid only until the next write or logbuf reset;
                 * giving python that view would dangle on the following render.
                 */
                .def("output",
                     [](const PrettySink & self) { return std::string(self.output()); },
                     "pretty-printed output flushed so far, as a str")

                .def("complete",
                     [](PrettySink & self) { self.complete(); },
                     "end the current record: emit the newline, then drain")

                .def("__repr__",
                     [](const PrettySink & self) {
                         return ("<PrettySink output_z="
                                 + std::to_string(self.output().size()) + ">");
                     });
        } /*pyindentlog2*/
    } /*namespace pp*/
} /*namespace xo*/

/* end pyindentlog2.cpp */
