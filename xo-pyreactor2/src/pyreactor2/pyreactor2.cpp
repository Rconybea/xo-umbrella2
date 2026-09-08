/* @file pyreactor2.cpp
 *
 * python bindings for xo-reactor2 (the facet-based reactor).
 *
 * Scaffold: the module builds, imports, and links against xo_reactor2, but
 * binds nothing yet.  Bindings get added here as the reactor2 surface settles.
 */

#include "pyreactor2.hpp"
/* not used yet -- included so the scaffold exercises the xo_reactor2 include
 * path and dependency edge rather than only declaring them in cmake
 */
#include <xo/reactor2/AbstractEventProcessor.hpp>
//#include <xo/pyprintjson/pyprintjson.hpp>
//#include <xo/pyreflect/pyreflect.hpp>
#include <pybind11/pybind11.h>

namespace xo {
    namespace py = pybind11;
    using xo::mm::AGCObject;

    namespace reactor {
        PYBIND11_MODULE(PYREACTOR2_MODULE_NAME(), m) {
            /* e.g. for TypeDescr */
            //PYREFLECT_IMPORT_MODULE(); //py::module_::import("pyreflect");
            //PYPRINTJSON_IMPORT_MODULE(); //py::module_::import("pyprintjson");

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.reactor2";

            py::class_<obj<AGCObject>>(m, "GCObject");
        } /*pyreactor2*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end pyreactor2.cpp */
