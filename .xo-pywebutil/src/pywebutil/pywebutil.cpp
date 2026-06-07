/* @file pywebutil.cpp */

#include "pywebutil.hpp"
#include "xo/webutil/HttpEndpointDescr.hpp"
#include "xo/webutil/StreamEndpointDescr.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/chrono.h>

namespace xo {
    //using xo::web::Alist;
    using xo::web::HttpEndpointDescr;
    //using xo::time::utc_nanos;
    namespace py = pybind11;

    namespace web {
        PYBIND11_MODULE(PYWEBUTIL_MODULE_NAME(), m) {
            //PYxxx_IMPORT_MODULE();

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.web_util";

            py::class_<HttpEndpointDescr>(m, "EndpointDescr")
                .def_property_readonly("uri_pattern", &HttpEndpointDescr::uri_pattern)
                .def("__repr__", &HttpEndpointDescr::display_string);

            py::class_<StreamEndpointDescr>(m, "StreamEndpointDescr")
                .def_property_readonly("uri_pattern", &StreamEndpointDescr::uri_pattern)
                .def("__repr__", &StreamEndpointDescr::display_string);
        } /*web*/
    } /*namespace web*/
} /*namespace xo*/

/* end pywebutil.cpp */
