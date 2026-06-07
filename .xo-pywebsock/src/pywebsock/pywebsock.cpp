/* @file pywebsock.cpp */

#include "pywebsock.hpp"
#include "xo/pywebutil/pywebutil.hpp"
#include "xo/websock/Webserver.hpp"
#include "xo/printjson/PrintJson.hpp"
//#include "web_util/EndpointDescr.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/chrono.h>

namespace xo {
    using xo::web::WebserverConfig;
    using xo::web::Webserver;
    using xo::web::Runstate;
    using xo::json::PrintJsonSingleton;
    using xo::rp;
    namespace py = pybind11;

    namespace web {
        PYBIND11_MODULE(PYWEBSOCK_MODULE_NAME(), m) {
            PYWEBUTIL_IMPORT_MODULE(); // = py::module_::import("pywebutil")

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.websock";

            py::enum_<Runstate>(m, "Runstate")
                .value("stopped", Runstate::stopped)
                .value("stop_requested", Runstate::stop_requested)
                .value("running", Runstate::running);

            py::class_<WebserverConfig>(m, "WebserverConfig")
                .def(py::init<uint32_t, bool, bool, bool>(),
                     py::arg("port"),
                     py::arg("tls_flag"),
                     py::arg("host_check_flag"),
                     py::arg("use_retry_flag"))
                .def_property_readonly("port", &WebserverConfig::port)
                .def_property_readonly("tls_flag", &WebserverConfig::tls_flag)
                .def_property_readonly("host_check_flag", &WebserverConfig::host_check_flag)
                .def_property_readonly("use_retry_flag", &WebserverConfig::use_retry_flag);

            py::class_<Webserver, rp<Webserver>>(m, "Webserver")
                .def_static("make",
                            [](WebserverConfig const & ws_config)
                                {
                                    return Webserver::make(ws_config,
                                                           PrintJsonSingleton::instance());
                                })
                .def_property_readonly("state", &Webserver::state)
                .def("register_http_endpoint", &Webserver::register_http_endpoint)
                .def("register_stream_endpoint", &Webserver::register_stream_endpoint)
                .def("start_webserver", &Webserver::start_webserver)
                .def("stop_webserver", &Webserver::stop_webserver)
                .def("join_webserver", &Webserver::join_webserver)
                .def("__repr__", &Webserver::display_string);

            m.def("make_webserver",
                  &Webserver::make);
        } /*pywebsock*/
    } /*web*/
} /*namespace xo*/

/* end pywebsock.cpp */
