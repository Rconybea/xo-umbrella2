/* @file pyarena.cpp
 *
 * python bindings for xo-arena's configuration types.
 *
 * SCOPE.  Configuration only: ArenaConfig, AllocHeaderConfig and the
 * AllocHeader word they describe.  DArena itself is not bound -- an arena is
 * handed to c++ code that allocates from it, and python has no use for a
 * handle it cannot allocate through.
 *
 * These types were originally registered by xo-pyindentlog2, because pybind11
 * permits exactly one registration per c++ type and that was the lowest python
 * module needing them (a PpConfig's logbuf is an ArenaConfig).  They live here
 * now; consumers reach them by importing this module, which is what
 * PYARENA_IMPORT_MODULE() is for.
 */

#include "pyarena.hpp"
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

    namespace mm {
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

        PYBIND11_MODULE(PYARENA_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.arena";

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
        } /*PYBIND11_MODULE*/
    } /*namespace mm*/
} /*namespace xo*/

/* end pyarena.cpp */
