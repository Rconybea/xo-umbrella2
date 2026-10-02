/* @file pywebsock.cpp */

#include "pywebsock.hpp"
#include <xo/websock/Webserver.hpp>
#include <xo/websock/cx/WebsockAppcx.hpp>
#include <xo/pyprintjson/pyprintjson.hpp>
#include <xo/pyreflect/pyreflect.hpp>
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/pywebutil/pywebutil.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <pybind11/chrono.h>
#include <memory>
#include <stdexcept>

namespace xo {
    using xo::web::WebserverConfig;
    using xo::web::Webserver;
    using xo::web::Runstate;
    using xo::rp;
    namespace py = pybind11;

    namespace web {
        namespace {
            /** Enforce at most one WebsockAppcx per python instance, as
             *  xo.printjson does for its context: this one installs printers
             *  into the process-wide printer table.
             *
             *  @return websock appcx, to be owned by python.
             **/
            std::unique_ptr<WebsockAppcx>
            configure_once(const WebsockConfig & cfg,
                           const ReflectAppcx & reflect_appcx,
                           const PrintJsonAppcx & printjson_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                if (s_configured) {
                    throw std::runtime_error
                        ("xo.websock.configure: already configured;"
                         " the json printer table is process-wide");
                }

                auto retval = std::make_unique<WebsockAppcx>(cfg, reflect_appcx, printjson_appcx);

                s_configured = true;

                return retval;
            }
        } /*namespace*/

        PYBIND11_MODULE(XO_PYWEBSOCK_MODULE_NAME(), m) {
            XO_PYWEBUTIL_IMPORT_MODULE(); // = py::module_::import("pywebutil")
            /* configure()'s second argument, PrintJsonAppcx, is registered by
             * xo.printjson; pybind11 permits one registration per c++ type
             */
            XO_PYPRINTJSON_IMPORT_MODULE();
            /* ... and its first, ReflectAppcx, by xo.reflect */
            XO_PYREFLECT_IMPORT_MODULE();

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
                .def_property_readonly("use_retry_flag", &WebserverConfig::use_retry_flag)
                .def_property_readonly("mount_origin", &WebserverConfig::mount_origin)
                .def("with_mount_origin", &WebserverConfig::with_mount_origin,
                     py::arg("dir"),
                     "copy of this config serving static files from dir");

            // ----------------------------------------------------------------
            // subsystem configuration and context.  A Webserver is made from
            // the context (.xo-backlog/xo-websock/issues/11)

            py::class_<WebsockConfig>(m, "WebsockConfig")
                .def(py::init<>(),
                     "configuration for the xo-websock subsystem (no settings yet)")
                .def("__repr__", [](const WebsockConfig &) {
                        return std::string("<WebsockConfig>"); });

            py::class_<WebsockAppcx>(m, "WebsockAppcx")
                .def("config", &WebsockAppcx::config,
                     py::return_value_policy::reference_internal,
                     "the WebsockConfig this context was established with")
                .def("__repr__", [](const WebsockAppcx &) {
                        return std::string("<WebsockAppcx>"); });

            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("reflect_appcx"),
                  py::arg("printjson_appcx"),
                  py::keep_alive<0, 2>(),
                  py::keep_alive<0, 3>(),
                  "establish process-wide context for the xo-websock subsystem.");

            py::class_<Webserver, rp<Webserver>>(m, "Webserver")
                /* keep_alive: the server's PrintJson came from the context */
                .def_static("make", &Webserver::make,
                            py::arg("cx"),
                            py::arg("ws_config"),
                            py::keep_alive<0, 1>())
                .def_property_readonly("state", &Webserver::state)
                .def("register_http_endpoint", &Webserver::register_http_endpoint)
                .def("register_stream_endpoint", &Webserver::register_stream_endpoint)
                .def("unregister_http_endpoint", &Webserver::unregister_http_endpoint,
                     py::arg("uri_pattern"),
                     "remove the http endpoint registered with exactly uri_pattern;"
                     " False if none")
                .def("unregister_stream_endpoint", &Webserver::unregister_stream_endpoint,
                     py::arg("uri_pattern"),
                     "remove the stream endpoint registered with exactly uri_pattern;"
                     " False if none.  Its live subscriptions are ended: each client"
                     " gets {\"cmd\": \"unsubscribed\", \"reason\": \"endpoint removed\"}")
                .def("start_webserver", &Webserver::start_webserver)
                .def("stop_webserver", &Webserver::stop_webserver)
                .def("join_webserver", &Webserver::join_webserver)
                .def("__repr__", &Webserver::display_string);

            m.def("make_webserver",
                  &Webserver::make,
                  py::arg("cx"),
                  py::arg("ws_config"),
                  py::keep_alive<0, 1>());
        } /*pywebsock*/
    } /*web*/
} /*namespace xo*/

/* end pywebsock.cpp */
