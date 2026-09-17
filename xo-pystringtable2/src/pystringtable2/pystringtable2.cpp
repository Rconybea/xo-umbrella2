/* @file pystringtable2.cpp */

// note: need pystringtable2/ here bc pystringtable2.hpp is generated,
//       located in build directory
#include "pystringtable2.hpp"
#include <xo/pyfacet/pyfacet.hpp>
#include <xo/pyindentlog2/pyindentlog2.hpp>
#include <xo/stringtable2/cx/Stringtable2Appcx.hpp>
#include <xo/stringtable2/String.hpp>
#include <xo/pyarena/CollectPools.hpp>
#include <xo/facet/ObjectHandle.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <pybind11/stl.h>   /* std::vector<MemorySizeInfo>, for visit_pools */
#include <string_view>

namespace xo {
    namespace py = pybind11;

    using xo::facet::DObjectHandle;
    using xo::facet::with_facet;
    using xo::mm::AAllocator;
    using xo::mm::AllocFlywheel;
    using xo::pp::TempPpSink;
    using xo::pp::PpSink;
    using xo::print::APrintable;

    namespace scm {
        namespace {
            /** Enforce at most one Stringtable2Appcx per python instance.
                *
             *  @return stringtable2 appcx, to be owned by python.
             **/
            std::unique_ptr<Stringtable2Appcx>
            configure_once(const Stringtable2Config & cfg,
                           const FacetAppcx & facet_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                if (s_configured) {
                    throw std::runtime_error
                        ("xo.stringtable2.configure: already configured;"
                         " facet registration is process-wide");
                }

                auto retval = std::make_unique<Stringtable2Appcx>(cfg, facet_appcx);

                s_configured = true;

                return retval;
            }

            /* no configure_all() here: that convenience belongs at the top
             * of the python stack, not on each subsystem.
             */

            /** python's proxy for a DString living in some AllocFlywheel.
             *
             *  Keyed on APrintable, for the reason xo-pyobject2's HFloat gives:
             *  make_strong_ref() takes an obj<AFacet,DRepr>, and
             *  obj<ATop,DString> cannot be formed -- ITop_Any specializes only
             *  the variant -- so the handle has to name a facet DString
             *  actually implements.  DString now implements AReflectable too
             *  (.xo-backlog/reflectable2/issues/05), but pretty() below wants
             *  the printable one.
             *
             *  LIFETIME, same as every other handle: a strong root is pinned in
             *  the flywheel and is NOT released when the handle dies
             *  (.xo-backlog/pyobject2/issues/02), so a flywheel's root set
             *  grows monotonically and nothing python has touched can become
             *  collectable.
             **/
            using HString = DObjectHandle<APrintable, DString>;
        } /*namespace*/

        PYBIND11_MODULE(XO_PYSTRINGTABLE2_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.stringtable2";

            /* FacetAppcx (configure()'s second argument) is registered by
             * xo_pyfacet, and pybind11 permits exactly one registration per
             * c++ type, so we import rather than re-register: without this
             * configure()'s signature does not resolve.
             */
            XO_PYFACET_IMPORT_MODULE();
            /* PpSink -- String.pretty's argument type -- is registered by
             * xo_pyindentlog2, on the same one-registration-per-type rule
             */
            XO_PYINDENTLOG2_IMPORT_MODULE();

            // ----------------------------------------------------------------
            // subsystem configuration and context.
            //
            // Import registers types and nothing else.  A context is built only
            // by configure(), so python supplies configuration the same way a
            // c++ main() does.

            py::class_<Stringtable2Config>(m, "Stringtable2Config")
                .def(py::init<>(),
                     "configuration for the xo-stringtable2 subsystem."
                     "  Empty, and expected to stay so: a StringTable's"
                     " capacity belongs to whoever constructs one, not to the"
                     " application")
                .def("__repr__", [](const Stringtable2Config &) {
                        return std::string("<Stringtable2Config>"); });

            py::class_<Stringtable2Appcx>(m, "Stringtable2Appcx")
                /* what this context was actually configured with.  Without it
                 * configuration is write-only from python: a caller cannot
                 * confirm the values it passed took effect, and a test cannot
                 * assert it.
                 */
                .def("config", &Stringtable2Appcx::config,
                     py::return_value_policy::reference_internal,
                     "the Stringtable2Config this context was established with")
                /* Empty, and NOT a placeholder -- unlike ReflectAppcx and
                 * PrintJsonAppcx, this one is not waiting on a DArena
                 * refactor.  xo-stringtable2 owns no pool.  A StringTable does
                 * have pools and its own visit_pools, but it belongs to its
                 * constructor's caller, who is the one able to report it.
                 */
                .def("visit_pools", &xo::pyarena::collect_pools<Stringtable2Appcx>,
                     "always empty, since xo-stringtable2 owns no memory pool.")
                .def("__repr__", [](const Stringtable2Appcx &) {
                return std::string("<Stringtable2Appcx>");
            });

            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("facet_appcx"),
                  py::keep_alive<0, 2>(),
                  "establish an xo-stringtable2 context, and return it."
                  "  The caller owns it; when the last python reference goes,"
                  " so does the context."
                  "  Takes the context returned by xo.facet.configure(), which"
                  " owns the FacetRegistry this subsystem registers into."
                  "  Throws if already configured.");

            // ----------------------------------------------------------------
            // String -- a byte string, allocated from a flywheel's arena

            py::class_<HString>(m, "String")
                /* named factory rather than py::init: a DString is allocated
                 * from an arena, so it cannot be constructed without being told
                 * which one.  Mirrors Float.make in xo_pyobject2.
                 */
                .def_static("make",
                            [](rp<AllocFlywheel> fw, std::string_view sv) {
                                auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

                                return HString::make_strong_ref
                                    (fw, with_facet<APrintable>::mkobj
                                         (DString::from_view(alloc, sv)));
                            },
                            py::arg("flywheel"),
                            py::arg("value"),
                            "copy a string into @p flywheel's arena, pinned by a"
                            " strong root.  The argument is encoded utf-8 on the"
                            " way in, so what DString stores is BYTES")

                .def("value",
                     [](const HString & self) {
                         const DString * p = self._native().data();

                         /* (chars(), size()), not the operator string_view(),
                          * which stops at the first null: size_ is the
                          * authority on extent.  pybind decodes as utf-8, so a
                          * DString holding bytes that are not valid utf-8
                          * raises UnicodeDecodeError here rather than at
                          * make() -- reading is where the encoding is asserted.
                          */
                         return std::string_view(p->chars(), p->size());
                     },
                     "the stored bytes, decoded as utf-8")

                .def("__str__",
                     [](const HString & self) {
                         const DString * p = self._native().data();

                         return std::string_view(p->chars(), p->size());
                     })

                .def("__len__",
                     [](const HString & self) { return self._native().data()->size(); },
                     "size in BYTES.  Differs from len(s.value()) for multibyte"
                     " text -- DString is byte-extent, python str is codepoints")

                .def("capacity",
                     [](const HString & self) {
                         return self._native().data()->capacity();
                     },
                     "chars the arena reserved, INCLUDING the null terminator."
                     "  Always len+1 for a String built by make(); it earns its"
                     " keep only against DString::empty, which python cannot"
                     " reach yet")

                .def("pretty",
                     [](const HString & self, PpSink & sink) {
                         // calling .complete() would be broken for
                         // sinks that do not forward to a streambuf

                         sink.pp(self._native());
                     },
                     py::arg("sink"),
                     "pretty-print this string into sink")

                .def("__repr__",
                     [](const HString & self) {
                         /** proves TempPrettySink is available **/
                         static_assert(xo::carries_indentlog2_appcx<HString>);

                         return TempPpSink::pp2str(self._native());
                     });
        } /*pystringtable2*/
    } /*namespace scm*/
} /*namespace xo*/

/* end pystringtable2.cpp */
