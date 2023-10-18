/* @file pyprocess.cpp */

// note: need pyprocess/ here bc pyprocess.hpp is generated, located in build directory
#include "src/pyprocess/pyprocess.hpp"
#include "xo/pywebutil/pywebutil.hpp"
#include "xo/process/init_process.hpp"
#include "xo/process/UpxToConsole.hpp"
#include "xo/process/StochasticProcess.hpp"
#include "xo/process/BrownianMotion.hpp"
#include "xo/process/ExpProcess.hpp"
#include "xo/process/RealizationSource.hpp"
#include "xo/pyreactor/pyreactor.hpp"
#include "xo/reactor/EventStore.hpp"
#include "xo/reactor/PolyAdapterSink.hpp"
#include "xo/randomgen/random_seed.hpp"
#include "xo/randomgen/xoshiro256.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
    using xo::reactor::AbstractSink;
    using xo::reactor::AbstractEventStore;
    using xo::reactor::StructEventStore;
    using xo::reactor::PolyAdapterSink;
    using xo::json::PrintJsonSingleton;
    using xo::time::utc_nanos;
    using xo::rng::Seed;
    using xo::rng::xoshiro256ss;
    using xo::ref::rp;
    namespace py = pybind11;

    namespace process {
        PYBIND11_MODULE(PYPROCESS_MODULE_NAME(), m) {
            /* ensure process/ will be initialized */
            InitSubsys<S_process_tag>::require();
            /* ..and immediately perform init steps */
            Subsystem::initialize_all();

            /* e.g. py wrapper for xo::reactor::ReactorSource */
            PYREACTOR_IMPORT_MODULE();
            /* e.g. py wrapper for xo::web::EndpointDescr */
            PYWEBUTIL_IMPORT_MODULE();

            m.doc() = "pybind11 plugin for xo.process";

            m.def("make_brownian_motion",
                  [](utc_nanos start_tm,
                     double annual_volatility) {
                      Seed<xoshiro256ss> seed;

                      return BrownianMotion<xoshiro256ss>::make(start_tm,
                                                                annual_volatility,
                                                                seed);
                  },
                  "create new BrownianMotion instance");

            m.def("make_exponential_brownian_motion",
                  [](utc_nanos start_tm,
                     double start_value,
                     double annual_volatility) {
                      Seed<xoshiro256ss> seed;

                      return ExpProcess::make(start_value /*scale*/,
                                              BrownianMotion<xoshiro256ss>::make(start_tm,
                                                                                 annual_volatility,
                                                                                 seed));
                  },
                  py::arg("start_tm"), py::arg("start_value"), py::arg("annual_volatility"));

            py::class_<StochasticProcess<double>,
                       xo::ref::rp<StochasticProcess<double>>>(m, "StochasticProcess")
                .def_property_readonly("t0", &StochasticProcess<double>::t0)
                .def_property_readonly("t0_value", &StochasticProcess<double>::t0_value)
                .def("exterior_sample", &StochasticProcess<double>::exterior_sample)
                .def("__repr__", &StochasticProcess<double>::display_string);

            py::class_<BrownianMotion<xoshiro256ss>,
                       StochasticProcess<double>,
                       xo::ref::rp<BrownianMotion<xoshiro256ss>>>(m, "BrownianMotion");
            //.def("exterior_sample", &BrownianMotion<xoshiro256ss>::exterior_sample)
            //.def("__repr__", &BrownianMotion<xoshiro256ss>::display_string);

            py::class_<ExpProcess, StochasticProcess<double>,
                       xo::ref::rp<ExpProcess>>(m, "ExpProcess")
                .def_property_readonly("exponent_process",
                                       [](ExpProcess & self) {
                                           return self.exponent_process().promote();
                                       });

            m.def("make_tracer",
                  &RealizationTracer<double>::make);

            py::class_<RealizationTracer<double>,
                       xo::ref::rp<RealizationTracer<double>>>(m, "RealizationTracer");

            /* e.g.
             *   import datetime as dt
             *   t0=dt.datetime.now()
             *   ebm=pyprocess.make_exponential_brownian_motion(t0, 0.5)
             *   s=pyprocess.make_realization_source(ebm, dt.timedelta(seconds=1))
             */
            m.def("make_realization_source",
                  [](xo::ref::rp<StochasticProcess<double>> p,
                     xo::time::nanos sample_dt)
                      {
                          auto tracer = RealizationTracer<double>::make(p);

                          return RealizationSource<UpxEvent, double>::make(tracer,
                                                                           sample_dt);
                      });

            /* note: providing __repr__ changes printing behavior,
             *       but uses default printer for inherited std::pair<..>
             */
            py::class_<UpxEvent>(m, "UpxEvent")
                .def_property_readonly("tm", &UpxEvent::tm)
                .def_property_readonly("upx", &UpxEvent::upx)
                .def("__repr__", &UpxEvent::display_string);

            py::class_<RealizationSource<UpxEvent, double>,
                       reactor::ReactorSource,
                       xo::ref::rp<RealizationSource<UpxEvent, double>>>(m, "RealizationSource")
                .def_property_readonly("current_ev", &RealizationSource<UpxEvent, double>::current_ev);

            py::class_<UpxToConsole,
                       AbstractSink,
                       rp<UpxToConsole>>
                (m, "UpxToConsole");

            using UpxEventStore = StructEventStore<UpxEvent>;

            /* see also: KalmanFilterStateEventStore in [pyfilter/pyfilter.cpp]
             */
            py::class_<UpxEventStore,
                       AbstractSink,
                       AbstractEventStore,
                       rp<UpxEventStore>>
                (m, "UpxEventStore")
                .def_static("make", &UpxEventStore::make)
                .def_property_readonly("empty", &UpxEventStore::empty)
                .def_property_readonly("size", &UpxEventStore::size)
                .def("last_n", &UpxEventStore::last_n, py::arg("n"))
                .def("last_dt", &UpxEventStore::last_dt, py::arg("dt"));
            //.def("__repr__", &UpxEventStore::display_string);

            /* temporary -- to reveal compiler errors */
            using UpxAdapterSink = PolyAdapterSink<UpxEvent>;

            py::class_<UpxAdapterSink,
                       AbstractSink,
                       rp<UpxAdapterSink>>(m, "UpxAdapterSink")
                .def_static("make", &UpxAdapterSink::make);

            /* prints
             *   std::pair<utc_nanos, double>
             * pairs
             */
            m.def("make_realization_printer", &UpxToConsole::make);

#ifdef OBSOLETE
            /* this implementation fails -- looks like .so libraries
             * have separate typeinfo for std::pair<utc_nanos, double>
             * and don't find each other.
             */
            m.def("make_realization_printer2",
                  []
                      {
                          return reactor::TemporaryTest::realization_printer();
                      });
#endif

        } /*pyprocess*/
    } /*namespace process*/
} /*namespace xo*/

/* end pyprocess.cpp */
