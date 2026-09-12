/* @file pyarena.cpp
 *
 * python bindings for xo-arena's configuration and reporting types.
 *
 * SCOPE.  Configuration -- ArenaConfig, AllocHeaderConfig and the AllocHeader
 * word they describe -- plus MemorySizeInfo, which every visit_pools() in the
 * tree reports through.  DArena itself is not bound: an arena is handed to c++
 * code that allocates from it, and python has no use for a handle it cannot
 * allocate through.
 *
 * REPR.  Every __repr__ here goes through xo::pp::TempPpSink::pp2str(), so
 * python shows the text the type's own Prettifier produces and the two
 * cannot drift.  That sink is this thread's scratch sink: plain by
 * construction (it asks the factory for ColorSelect::k_plain, which is what a
 * repr wants -- no ansi escapes), and backed by the low-level fallback until
 * an application installs a factory, so `import xo_pyarena; repr(cfg)` works
 * with nothing configured.  Reaching for xo::pp::tostr0() instead would take
 * the DEFAULT PpStyle, and that colors tag names.
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
#include <xo/pyarena/CollectPools.hpp>
#include <xo/arena/MemorySizeInfo.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <xo/ppsink/TempPpSink.hpp>
#include <xo/ppsink/pretty.hpp>   /* PpSink::pp */
#include <pybind11/stl.h>   /* std::vector, std::optional */
#include <string>
#include <optional>
#include <cstdint>

namespace xo {
    namespace py = pybind11;

    using xo::mm::AllocHeader;
    using xo::mm::MemorySizeInfo;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::ArenaConfig;
    using xo::mm::ArenaNameStr;
    using xo::pp::TempPpSink;

    namespace mm {
        namespace {
            /** @p p as an int python can use, or None when the pool has no
             *  address range.  A bound `const void *` would arrive in python
             *  as a capsule, which is no use for arithmetic or for printing.
             **/
            std::optional<std::uintptr_t> addr_of(const void * p) {
                if (p)
                    return reinterpret_cast<std::uintptr_t>(p);

                return std::nullopt;
            }
        } /*namespace*/

        PYBIND11_MODULE(PYARENA_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.arena";

            // ----------------------------------------------------------------
            // memory reporting.  Types that own arenas expose
            //   visit_pools(fn) -> fn is called once per pool with one of these
            //
            // python is handed the c++ MemorySizeInfo itself, by value: every
            // member exposed below is owned (resource_name_ is a flatstring;
            // lo_/hi_ are addresses nothing dereferences).  The one member
            // that could not survive the callback -- detail_, the per-type
            // histogram, which points into the visiting frame -- is therefore
            // not exposed, and xo::pyarena::collect_pools() nulls it.  See
            // CollectPools.hpp.

            py::class_<MemorySizeInfo>(m, "MemorySizeInfo")
                /* the empty report.  Instances normally arrive from a
                 * visit_pools() callback rather than being built here.
                 */
                .def(py::init<>(), "an empty report -- all sizes zero")
                /* flatstring, so not directly convertible: python wants str */
                .def_property_readonly("name",
                                       [](const MemorySizeInfo & x) {
                                           return std::string(x.resource_name_.c_str());
                                       },
                                       "name of the pool being reported")
                /* four numbers, and they mean four different things */
                .def_readonly("used", &MemorySizeInfo::used_,
                              "bytes in use, excluding waste")
                .def_readonly("allocated", &MemorySizeInfo::allocated_,
                              "bytes allocated, including waste (e.g. empty hash slots)")
                .def_readonly("committed", &MemorySizeInfo::committed_,
                              "bytes backed by physical memory")
                .def_readonly("reserved", &MemorySizeInfo::reserved_,
                              "address space obtained, whether or not committed")
                .def_property_readonly("lo",
                                       [](const MemorySizeInfo & x) {
                                           return addr_of(x.lo_);
                                       },
                                       "start address, or None")
                .def_property_readonly("hi",
                                       [](const MemorySizeInfo & x) {
                                           return addr_of(x.hi_);
                                       },
                                       "end address, or None")
                /* through the type's own Prettifier, like the config types
                 * below -- so python shows the text c++ does
                 */
                .def("__repr__",
                     [](const MemorySizeInfo & x) { return TempPpSink::pp2str(x); });

            py::class_<AllocHeader>(m, "AllocHeader")
                .def(py::init<AllocHeader::repr_type>(), py::arg("repr"))
                .def_readwrite("repr", &AllocHeader::repr_)
                .def("__repr__",
                     [](const AllocHeader & x) { return TempPpSink::pp2str(x); });

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
                     [](const AllocHeaderConfig & x) { return TempPpSink::pp2str(x); });

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
                        cfg.name_ = ArenaNameStr::from_cstr(name.c_str());
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
                     [](const ArenaConfig & x) { return TempPpSink::pp2str(x); });
        } /*PYBIND11_MODULE*/
    } /*namespace mm*/
} /*namespace xo*/

/* end pyarena.cpp */
