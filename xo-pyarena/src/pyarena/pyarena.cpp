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
#include <xo/pyarena/PoolInfo.hpp>
#include <xo/arena/MemorySizeInfo.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <xo/ppsink/FlatSink.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/ppsink/pretty.hpp>   /* PpSink::pp */
#include <pybind11/stl.h>   /* std::vector, std::optional */
#include <sstream>
#include <string>

namespace xo {
    namespace py = pybind11;

    using xo::mm::AllocHeader;
    using xo::mm::MemorySizeDetail;
    using xo::mm::MemorySizeInfo;
    using xo::pyarena::PoolInfo;
    using xo::mm::AllocHeaderConfig;
    using xo::mm::ArenaConfig;
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;

    namespace mm {
        namespace {
            /** render @p x for __repr__, through its own Prettifier -- so
             *  python shows the same text c++ does, and the two cannot drift.
             *
             *  FlatSink, not TempPrettySink::pp2str: that lives in
             *  xo-indentlog2, which is ABOVE this module in the subsystem list
             *  (xo-pyarena 13, xo-indentlog2 14), and it needs the indentlog2
             *  context configured -- while `import xo_pyarena; repr(cfg)`
             *  must keep working on its own.  A repr wants one line anyway,
             *  which is exactly what FlatSink gives.
             *
             *  Spelled out rather than xo::pp::tostr0(), which is otherwise
             *  the right tool for this tier: it takes the default PpStyle,
             *  and that colors tag names.
             **/
            template <typename T>
            std::string pp2str(const T & x) {
                std::stringbuf buf;
                {
                    FlatSink sink(PpStyle::plain(), &buf);

                    sink.pp(x);
                }
                return buf.str();
            }
        } /*namespace*/

        PYBIND11_MODULE(PYARENA_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.arena";

            // ----------------------------------------------------------------
            // memory reporting.  Types that own arenas expose
            //   visit_pools(fn) -> fn is called once per pool with one of these
            //
            // The python object is an owning snapshot (xo::pyarena::PoolInfo),
            // not the c++ MemorySizeInfo it was taken from.  That type reports
            // through a string_view and a bare pointer into the visiting
            // frame, so it cannot outlive the callback -- a rule python code
            // has no way to keep.  See PoolInfo.hpp.

            py::class_<MemorySizeDetail>(m, "MemorySizeDetail")
                .def(py::init<>())
                .def_property_readonly("tseq",
                                       [](const MemorySizeDetail & x) {
                                           return x.tseq_.seqno();
                                       },
                                       "typeseq identifying the c++ type counted here"
                                       " (-1 in the leading totals entry)")
                .def_readonly("n_alloc", &MemorySizeDetail::n_alloc_,
                              "number of instances")
                .def_readonly("z_alloc", &MemorySizeDetail::z_alloc_,
                              "bytes used by those instances")
                .def("__repr__",
                     [](const MemorySizeDetail & x) {
                         return ("<MemorySizeDetail tseq=" + std::to_string(x.tseq_.seqno())
                                 + " n_alloc=" + std::to_string(x.n_alloc_)
                                 + " z_alloc=" + std::to_string(x.z_alloc_) + ">");
                     });

            py::class_<PoolInfo>(m, "MemorySizeInfo")
                /* the empty report.  Instances normally arrive from a
                 * visit_pools() callback rather than being built here.
                 */
                .def(py::init<>(), "an empty report -- all sizes zero")
                .def_readonly("name", &PoolInfo::name_,
                              "name of the pool being reported")
                /* four numbers, and they mean four different things */
                .def_readonly("used", &PoolInfo::used_,
                              "bytes in use, excluding waste")
                .def_readonly("allocated", &PoolInfo::allocated_,
                              "bytes allocated, including waste (e.g. empty hash slots)")
                .def_readonly("committed", &PoolInfo::committed_,
                              "bytes backed by physical memory")
                .def_readonly("reserved", &PoolInfo::reserved_,
                              "address space obtained, whether or not committed")
                .def_readonly("lo", &PoolInfo::lo_,
                              "start address, or None")
                .def_readonly("hi", &PoolInfo::hi_,
                              "end address, or None")
                .def_readonly("detail", &PoolInfo::detail_,
                              "per-type histogram, empty when the pool keeps none."
                              "  detail[0] is the total across types")
                .def("__repr__",
                     [](const PoolInfo & x) {
                         return ("<MemorySizeInfo " + x.name_
                                 + " used=" + std::to_string(x.used_)
                                 + " committed=" + std::to_string(x.committed_)
                                 + " reserved=" + std::to_string(x.reserved_) + ">");
                     });

            py::class_<AllocHeader>(m, "AllocHeader")
                .def(py::init<AllocHeader::repr_type>(), py::arg("repr"))
                .def_readwrite("repr", &AllocHeader::repr_)
                .def("__repr__",
                     [](const AllocHeader & x) { return pp2str(x); });

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
                     [](const AllocHeaderConfig & x) { return pp2str(x); });

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
                     [](const ArenaConfig & x) { return pp2str(x); });
        } /*PYBIND11_MODULE*/
    } /*namespace mm*/
} /*namespace xo*/

/* end pyarena.cpp */
