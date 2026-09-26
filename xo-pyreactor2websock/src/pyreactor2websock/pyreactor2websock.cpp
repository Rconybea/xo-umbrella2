/** @file pyreactor2websock.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "pyreactor2websock.hpp"
#include <xo/pyreactor/pyreactor.hpp>
#include <xo/reactor2websock/reactor_endpoints.hpp>
#include <xo/reactor/AbstractSource.hpp>
#include <xo/reactor/EventStore.hpp>
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/pywebutil/pywebutil.hpp>
#include <xo/pyutil/pyutil.hpp>

namespace xo {
    using xo::reactor::AbstractSource;
    using xo::reactor::AbstractEventStore;
    using xo::json::PrintJsonSingleton;
    namespace py = pybind11;

    namespace web {
        PYBIND11_MODULE(XO_PYREACTOR2WEBSOCK_MODULE_NAME(), m) {
            XO_PYREACTOR_IMPORT_MODULE();  // AbstractSource, AbstractEventStore
            XO_PYWEBUTIL_IMPORT_MODULE();  // StreamEndpointDescr, HttpEndpointDescr

            /* module docstring */
            m.doc() = ("pybind11 plugin for xo.reactor2websock:"
                       " web endpoints for reactor sources and event stores");

            /* Were methods on xo.reactor.AbstractSource and
             * xo.reactor.AbstractEventStore until 2026-09-26.  Moved here,
             * as module functions taking the source/store, so that xo-reactor
             * need not know about the web.  See
             * .xo-backlog/xo-websock/issues/02.
             */
            m.def("stream_endpoint_descr",
                  [](rp<AbstractSource> src, std::string const & url_prefix) {
                      return stream_endpoint_descr(src, url_prefix);
                  },
                  py::arg("src"), py::arg("url_prefix"),
                  "websocket stream endpoint delivering the events src produces;"
                  " register with Webserver.register_stream_endpoint");

            m.def("http_endpoint_descr",
                  [](rp<AbstractEventStore> store, std::string const & url_prefix) {
                      return http_endpoint_descr(store,
                                                 PrintJsonSingleton::instance(),
                                                 url_prefix);
                  },
                  py::arg("store"), py::arg("url_prefix"),
                  "http endpoint serving a json snapshot of store at"
                  " url_prefix + '/snap'; register with"
                  " Webserver.register_http_endpoint");
        } /*pyreactor2websock*/
    } /*namespace web*/
} /*namespace xo*/

/* end pyreactor2websock.cpp */
