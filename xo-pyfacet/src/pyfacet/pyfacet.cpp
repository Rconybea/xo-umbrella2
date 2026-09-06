/* @file pyfacet.cpp
 *
 * python bindings for xo-facet (the faceted object model).
 */

#include "pyfacet.hpp"
#include <xo/pyindentlog2/pyindentlog2.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/pyutil/pyutil.hpp>

namespace xo {
    namespace py = pybind11;

    using xo::mm::AllocFlywheel;
    using xo::mm::ArenaConfig;
    using xo::pp::PpSink;

    namespace facet {
        PYBIND11_MODULE(PYFACET_MODULE_NAME(), m) {
            /* for ArenaConfig (make_app's arguments) and PpSink (pretty's).
             * Both are registered there, and pybind11 permits exactly one
             * registration per c++ type.
             */
            PYINDENTLOG2_IMPORT_MODULE();

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.facet";

            // ----------------------------------------------------------------
            // AllocFlywheel -- consolidated arena + root set, the thing that
            // lets a python handle keep a fomo object alive across a gc cycle.

            /* held by rp<>, not unique_ptr: AllocFlywheel inherits
             * Displayable -> Refcount, so it carries its own intrusive count.
             * pyutil.hpp declares the holder.
             */
            py::class_<AllocFlywheel, rp<AllocFlywheel>>(m, "AllocFlywheel")
                /* named factory rather than py::init, mirroring c++: an
                 * AllocFlywheel is heap-allocated by make_app() and cannot be
                 * constructed any other way (it is neither copyable nor
                 * movable -- Refcount's atomic member deletes both).
                 */
                .def_static("make_app", &AllocFlywheel::make_app,
                            py::arg("storage_cfg"),
                            py::arg("strong_root_cfg"),
                            py::arg("weak_root_cfg"),
                            "create a flywheel: primary arena, strong root set,"
                            " weak root set")

                /* renders into a sink supplied by the caller -- typically an
                 * xo_pyindentlog2.PrettySink.  Declared as PpSink & so any
                 * sink works, not just PrettySink.
                 */
                .def("pretty",
                     [](const AllocFlywheel & self, PpSink & sink) {
                         self.pretty(sink);
                     },
                     py::arg("sink"),
                     "pretty-print this flywheel into sink");
        } /*pyfacet*/
    } /*namespace facet*/
} /*namespace xo*/

/* end pyfacet.cpp */
