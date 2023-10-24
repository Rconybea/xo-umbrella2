/* @file pyreactor.cpp */

#include "pyreactor.hpp"
#include "xo/pyprintjson/pyprintjson.hpp"
#include "xo/pyreflect/pyreflect.hpp"

#include "xo/reactor/Reactor.hpp"
#include "xo/reactor/ReactorSource.hpp"
#include "xo/reactor/EventStore.hpp"
#include "xo/reactor/Sink.hpp"
#include "xo/webutil/StreamEndpointDescr.hpp"
//#include "time/Time.hpp"

//#include "xo/pyutil/pytime.hpp"
#include "xo/pyutil/pyutil.hpp"

//#include <pybind11/pybind11.h>
//#include <pybind11/chrono.h>
#include <pybind11/stl.h>

namespace xo {
    using xo::json::PrintJsonSingleton;
    using xo::fn::CallbackId;
    using xo::ref::Refcount;
    using xo::ref::rp;
    using xo::time::utc_nanos;
    using xo::tostr;
    namespace py = pybind11;

    namespace reactor {
        PYBIND11_MODULE(PYREACTOR_MODULE_NAME(), m) {
            /* e.g. for TypeDescr */
            PYREFLECT_IMPORT_MODULE(); //py::module_::import("pyreflect");
            PYPRINTJSON_IMPORT_MODULE(); //py::module_::import("pyprintjson");

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.reactor";

            m.def("time2str", [](utc_nanos tm) { return tostr(tm); });

            /* TODO: if we write pycallback/,  then CallbackId wrapper belongs there */
            py::class_<CallbackId>(m, "CallbackId");

            py::class_<AbstractEventProcessor,
                       xo::ref::rp<AbstractEventProcessor>>(m, "AbstractEventProcessor")
                .def_property("name",
                              &AbstractEventProcessor::name,
                              &AbstractEventProcessor::set_name)
                .def("reference_counter", [](AbstractEventProcessor const & x) { return x.reference_counter(); })
                .def("memory_address", [](AbstractEventProcessor const & x) { return (void*)&x; })
                .def("map_network", [](AbstractEventProcessor & x) { return AbstractEventProcessor::map_network(&x); })
                .def("__repr__", &AbstractEventProcessor::display_string);

            py::class_<AbstractSource,
                       AbstractEventProcessor,
                       xo::ref::rp<AbstractSource>>(m, "AbstractSource")
                .def_property_readonly("source_ev_type", &AbstractSource::source_ev_type)
                .def_property_readonly("is_volatile", &AbstractSource::is_volatile)
                .def_property_readonly("n_out_ev", &AbstractSource::n_out_ev)
                .def_property_readonly("n_queued_out_ev", &AbstractSource::n_queued_out_ev)
                .def("attach_sink", &AbstractSource::attach_sink)
                .def("detach_sink", &AbstractSource::detach_sink)
                /* editor bait: websock_endpoint_descr */
                .def("stream_endpoint_descr", &AbstractSource::stream_endpoint_descr)
                .def("deliver_one", &AbstractSource::deliver_one)
                .def("deliver_n", &AbstractSource::deliver_n,
                     py::arg("n"));

            py::class_<AbstractSink,
                       AbstractEventProcessor,
                       xo::ref::rp<AbstractSink>>(m, "AbstractSink")
                //.cdef("__repr__", &AbstractSink::display_string)
                .def_property_readonly("sink_ev_type", &AbstractSink::sink_ev_type)
                .def_property_readonly("n_in_ev", &AbstractSink::n_in_ev)
                .def("attach_source", &AbstractSink::attach_source);

            py::class_<ReactorSource,
                       AbstractSource,
                       xo::ref::rp<ReactorSource>>
                (m, "ReactorSource")
                .def_property_readonly("is_empty", &ReactorSource::is_empty)
                .def_property_readonly("is_nonempty", &ReactorSource::is_nonempty)
                .def_property_readonly("is_exhausted", &ReactorSource::is_exhausted)
                .def_property_readonly("sim_current_tm", &ReactorSource::sim_current_tm)
                .def_property("debug_sim_flag",
                              &ReactorSource::debug_sim_flag,
                              &ReactorSource::set_debug_sim_flag);

            py::class_<AbstractEventStore, rp<AbstractEventStore>>
                (m, "AbstractEventStore")
                .def_property_readonly("empty", &AbstractEventStore::empty)
                .def_property_readonly("size", &AbstractEventStore::size)
                .def("http_snapshot",
                     [](AbstractEventStore & self) {
                         std::stringstream ss;
                         self.http_snapshot(PrintJsonSingleton::instance(), &ss);
                         return ss.str();
                     })
                .def("http_endpoint_descr",
                     [](AbstractEventStore & self, std::string const & url_prefix) {
                         return self.http_endpoint_descr(PrintJsonSingleton::instance(), url_prefix);
                     },
                     py::arg("url_prefix"))
                .def("clear",
                     &AbstractEventStore::clear);

            py::class_<Reactor,
                       rp<Reactor>>
                (m, "Reactor")
                .def("add_source",
                     [](Reactor & self, rp<ReactorSource> src) {
                         return self.add_source(src.borrow());
                     })
                .def("remove_source",
                     [](Reactor & self, rp<ReactorSource> src) {
                         return self.remove_source(src.borrow());
                     })
                .def("run_one", &Reactor::run_one)
                .def("run_n",   &Reactor::run_n, py::arg("n"));

#ifdef NOT_IN_USE  // trying removed code in ProcessPy.cpp instead for now
            /* prints
             *   std::pair<utc_nanos, double>
             * pairs
             */
            m.def("make_realization_printer",
                  []
                      {
                          return new SinkToConsole<std::pair<utc_nanos, double>>();
                      });

            py::class_<SinkToConsole<std::pair<utc_nanos, double>>,
                       AbstractSink,
                       xo::ref::rp<SinkToConsole<std::pair<utc_nanos, double>>>>
                (m, "SinkToConsole");
#endif
        } /*pyreactor*/
    } /*namespace reactor*/
} /*namespace xo*/

/* end pyreactor.cpp */
