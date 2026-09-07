/* @file pyobject2.cpp
 *
 * python bindings for xo-object2 (the fomo object representations).
 *
 * SHAPE.  One python class per REPRESENTATION, not per facet.  A class is
 * assembled from per-facet binder templates once those exist (see
 * .xo-backlog/pyobject2/spec.md); until then the members are written out here.
 *
 * LIFETIME.  An object is held through DObjectHandle, which pins a strong root
 * in the AllocFlywheel it was allocated from, so the flywheel keeps the backing
 * memory alive for as long as python holds the handle.  NB the root is not yet
 * released when the handle dies (pyobject2 ticket 02): a flywheel's strong root
 * set therefore grows monotonically, and nothing python has touched can become
 * collectable.
 */

#include "pyobject2.hpp"
#include <xo/pyfacet/pyfacet.hpp>
#include <xo/pyindentlog2/pyindentlog2.hpp>
#include <xo/object2/Float.hpp>
#include <xo/object2/SetupObject2.hpp>
#include <xo/facet/ObjectHandle.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/printable2/Printable.hpp>
#include <xo/indentlog2/TempPrettySink.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/alloc2/arena/IAllocator_DArena.hpp>
#include <xo/pyutil/pyutil.hpp>

namespace xo {
    namespace py = pybind11;

    using xo::facet::DObjectHandle;
    using xo::facet::with_facet;
    using xo::mm::AAllocator;
    using xo::mm::AllocFlywheel;
    using xo::pp::PpSink;
    using xo::print::APrintable;

    namespace scm {
        namespace {
            /** python's proxy for a DFloat living in some AllocFlywheel.
             *
             *  Keyed on APrintable, which value() below does not use: value()
             *  is a plain DFloat member, reachable through _native().data()
             *  whatever the facet.  But make_strong_ref() takes an
             *  obj<AFacet,DRepr>, and obj<ATop,DFloat> cannot be formed --
             *  ITop_Any specializes only the variant -- so the handle has to
             *  name a facet DFloat actually implements.
             **/
            using HFloat = DObjectHandle<APrintable, DFloat>;
        } /*namespace*/

        PYBIND11_MODULE(PYOBJECT2_MODULE_NAME(), m) {
            /* AllocFlywheel and PpSink are registered by these modules, and
             * pybind11 permits exactly one registration per c++ type
             */
            PYFACET_IMPORT_MODULE();
            PYINDENTLOG2_IMPORT_MODULE();

            /* facet implementations for the object2 representations.
             * Unnecessary: guaranteed by HFloat
             */
            //SetupObject2::register_facets();

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.object2";

            // ----------------------------------------------------------------
            // Float -- boxed double, allocated from a flywheel's arena

            py::class_<HFloat>(m, "Float")
                /* named factory rather than py::init: a DFloat is allocated
                 * from an arena, so it cannot be constructed without being told
                 * which one.  Mirrors AllocFlywheel.make_app in xo_pyfacet.
                 */
                .def_static("make",
                            [](rp<AllocFlywheel> fw, double x) {
                                auto alloc = with_facet<AAllocator>::mkobj(&fw->storage());

                                return HFloat::make_strong_ref
                                    (fw, with_facet<APrintable>::mkobj(DFloat::_box(alloc, x)));
                            },
                            py::arg("flywheel"),
                            py::arg("value"),
                            "box a double in @p flywheel's arena, pinned by a strong root")

                .def("value",
                     [](const HFloat & self) { return self._native().data()->value(); },
                     "the boxed double")

                .def("pretty",
                     [](const HFloat & self, PpSink & sink) {
                         // calling .complete() would be broken for
                         // sinks that do not forward to a streambuf

                         sink.pp(self._native());
                         //self._native().pretty(sink);
                     },
                     py::arg("sink"),
                     "pretty-print this float into sink")

                .def("__repr__",
                     [](const HFloat & self) {
                         /** proves TempPrettySink is available **/
                         static_assert(xo::carries_indentlog2<HFloat>);

                         return xo::pp::TempPrettySink::pp2str(self._native());
                     });

        } /*PYBIND11_MODULE*/
    } /*namespace scm*/
} /*namespace xo*/

/* end pyobject2.cpp */
