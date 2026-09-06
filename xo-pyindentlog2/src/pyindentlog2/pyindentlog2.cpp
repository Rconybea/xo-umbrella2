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
#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/indentlog2/print/PpConfig.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/arena/AllocHeaderConfig.hpp>
#include <xo/arena/AllocHeader.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <string>

namespace xo {
    namespace py = pybind11;

    using xo::mm::AllocHeader;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::ArenaConfig;

    namespace pp {
        namespace {
            /** hex, for the bitfield-valued reprs.  Rendered by hand rather
             *  than through a stringstream: xo is migrating off std::ostream
             *  (see the ostream-containment milestone).  xo::pp::hex renders
             *  a single BYTE into a sink; this is a 64-bit word into a str.
             **/
            std::string to_hex(std::uint64_t x) {
                static const char * s_digits = "0123456789abcdef";

                std::string buf;
                for (int shift = 60; shift >= 0; shift -= 4)
                    buf.push_back(s_digits[(x >> shift) & 0xf]);

                return "0x" + buf;
            }
        } /*namespace*/

        PYBIND11_MODULE(PYINDENTLOG2_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.indentlog2";

            // ----------------------------------------------------------------
            // xo-arena configuration types.
            //
            // These are xo::mm types, not xo::pp ones.  They live in this
            // module because pybind11 permits exactly one registration per c++
            // type, and xo-pyindentlog2 is the LOWEST python module that needs
            // them -- PpConfig's logbuf is an ArenaConfig.  Expect them to move
            // again if an xo-pyarena is ever created.

            py::class_<AllocHeader>(m, "AllocHeader")
                .def(py::init<AllocHeader::repr_type>(), py::arg("repr"))
                .def_readwrite("repr", &AllocHeader::repr_)
                .def("__repr__",
                     [](const AllocHeader & x) {
                         return "<AllocHeader " + to_hex(x.repr_) + ">";
                     });

            py::class_<AllocHeaderConfig>(m, "AllocHeaderConfig")
                /* defaults mirror the c++ member initializers, so
                 * AllocHeaderConfig() agrees across the two languages
                 */
                .def(py::init<std::uint32_t, std::uint8_t,
                              std::uint8_t, std::uint8_t, std::uint8_t>(),
                     py::arg("guard_z") = 0,
                     py::arg("guard_byte") = 0xfd,
                     py::arg("tseq_bits") = 24,
                     py::arg("age_bits") = 8,
                     py::arg("size_bits") = 32)

                .def_readwrite("guard_z", &AllocHeaderConfig::guard_z_)
                .def_readwrite("guard_byte", &AllocHeaderConfig::guard_byte_)
                .def_readwrite("tseq_bits", &AllocHeaderConfig::tseq_bits_)
                .def_readwrite("age_bits", &AllocHeaderConfig::age_bits_)
                .def_readwrite("size_bits", &AllocHeaderConfig::size_bits_)

                .def("mkheader", &AllocHeaderConfig::mkheader,
                     py::arg("t"), py::arg("a"), py::arg("z"))

                .def("tseq_mask", &AllocHeaderConfig::tseq_mask)
                .def("age_mask", &AllocHeaderConfig::age_mask)
                .def("size_mask", &AllocHeaderConfig::size_mask)
                .def("max_age", &AllocHeaderConfig::max_age)
                .def("is_size_enabled", &AllocHeaderConfig::is_size_enabled)

                .def("tseq", &AllocHeaderConfig::tseq, py::arg("hdr"))
                .def("age", &AllocHeaderConfig::age, py::arg("hdr"))
                .def("size", &AllocHeaderConfig::size, py::arg("hdr"))
                .def("size_with_padding", &AllocHeaderConfig::size_with_padding,
                     py::arg("hdr"))

                .def("is_forwarding_tseq", &AllocHeaderConfig::is_forwarding_tseq,
                     py::arg("hdr"))
                .def("mark_forwarding_tseq", &AllocHeaderConfig::mark_forwarding_tseq,
                     py::arg("hdr"))

                .def("__repr__",
                     [](const AllocHeaderConfig & x) {
                         return ("<AllocHeaderConfig tseq_bits="
                                 + std::to_string(x.tseq_bits_)
                                 + " age_bits=" + std::to_string(x.age_bits_)
                                 + " size_bits=" + std::to_string(x.size_bits_)
                                 + " guard_z=" + std::to_string(x.guard_z_)
                                 + ">");
                     });

            py::class_<ArenaConfig>(m, "ArenaConfig")
                /* NB a single keyword ctor, rather than this plus py::init<>():
                 * every argument has a default, so it already covers
                 * ArenaConfig().  ArenaConfig deliberately declares no c++
                 * ctors (it wants designated initializers), so this is
                 * assembled field by field.
                 */
                .def(py::init([](std::string name,
                                 std::size_t size,
                                 std::size_t hugepage_z,
                                 bool store_header_flag,
                                 AllocHeaderConfig header,
                                 bool debug_flag)
                    {
                        ArenaConfig cfg;
                        cfg.name_ = std::move(name);
                        cfg.size_ = size;
                        cfg.hugepage_z_ = hugepage_z;
                        cfg.store_header_flag_ = store_header_flag;
                        cfg.header_ = header;
                        cfg.debug_flag_ = debug_flag;
                        return cfg;
                    }),
                     py::arg("name") = std::string(),
                     py::arg("size") = 0,
                     py::arg("hugepage_z") = 2 * 1024 * 1024,
                     py::arg("store_header_flag") = false,
                     py::arg("header") = AllocHeaderConfig(),
                     py::arg("debug_flag") = false)

                .def_readwrite("name", &ArenaConfig::name_)
                .def_readwrite("size", &ArenaConfig::size_)
                .def_readwrite("hugepage_z", &ArenaConfig::hugepage_z_)
                .def_readwrite("store_header_flag", &ArenaConfig::store_header_flag_)
                .def_readwrite("header", &ArenaConfig::header_)
                .def_readwrite("debug_flag", &ArenaConfig::debug_flag_)

                /* fluent setters, mirroring the c++ side.  These COPY -- each
                 * returns a new ArenaConfig, it does not mutate self.
                 */
                .def("with_name", &ArenaConfig::with_name, py::arg("name"))
                .def("with_size", &ArenaConfig::with_size, py::arg("size"))
                .def("with_store_header_flag", &ArenaConfig::with_store_header_flag,
                     py::arg("flag"))

                .def("__repr__",
                     [](const ArenaConfig & x) {
                         return ("<ArenaConfig name=\'" + x.name_
                                 + "\' size=" + std::to_string(x.size_)
                                 + " hugepage_z=" + std::to_string(x.hugepage_z_)
                                 + " store_header_flag="
                                 + (x.store_header_flag_ ? "True" : "False")
                                 + " debug_flag="
                                 + (x.debug_flag_ ? "True" : "False")
                                 + ">");
                     });

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
