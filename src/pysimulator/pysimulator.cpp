/* @file pysimulator.cpp */

#include "pysimulator.hpp"
#include "xo/pyreactor/pyreactor.hpp"

#include "xo/simulator/Simulator.hpp"
#include "xo/simulator/SourceTimestamp.hpp"
#include "xo/simulator/TimeSlip.hpp"
#include "xo/pyutil/pyutil.hpp"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/chrono.h>

namespace xo {
    using xo::reactor::Reactor;
    //using xo::scope;
    namespace py = pybind11;

    namespace sim {
        PYBIND11_MODULE(PYSIMULATOR_MODULE_NAME(), m) {
            /* e.g. for Reactor */
            PYREACTOR_IMPORT_MODULE(); //py::module_::import("pyreactor")

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.simulator";

            m.def("make_simulator",
                  []() {
                      return xo::sim::Simulator::make(xo::time::timeutil::epoch());
                  },
                  "create new Simulator instance");

            py::class_<TimeSlip>(m, "TimeSlip")
                .def_property_readonly("sim_tm", &TimeSlip::sim_tm)
                .def_property_readonly("real_tm", &TimeSlip::real_tm)
                // .def("__repr__", &TimeSlip::display_string)  // TODO
                ;

            py::class_<Simulator,
                       Reactor,
                       xo::ref::intrusive_ptr<Simulator>>(m, "Simulator")
                .def_static("make", []() { return xo::sim::Simulator::make(xo::time::timeutil::epoch()); })
                .def_property_readonly("start_tm", &Simulator::t0)
                .def_property_readonly("last_tm", &Simulator::last_tm)
                .def_property_readonly("n_event", &Simulator::n_event)
                .def_property_readonly("is_exhausted", &Simulator::is_exhausted)
                .def("next_tm", &Simulator::next_tm)
                .def("next_src", &Simulator::next_src)
                .def("timeslip", &Simulator::timeslip)
                .def("throttled_event_dt", &Simulator::throttled_event_dt)
                .def("heap_contents", &Simulator::heap_contents)
                .def("log_heap_contents",
                     [](Simulator & self) {
                         scope log(XO_LITERAL(log_level::always, "pysimulator", ".log_heap_contents"));
                         self.log_heap_contents(&log);
                     })
                .def("__repr__", &Simulator::display_string);

            py::class_<SourceTimestamp>(m, "SourceTimestamp")
                .def("__repr__", &SourceTimestamp::display_string);

        } /*pysimulator*/
    } /*namespace sim*/
} /*namespace xo*/

/* end pysimulator.cpp */
