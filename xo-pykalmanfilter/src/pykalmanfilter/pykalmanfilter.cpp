/* @file pykalmanfilter.cpp */

#include "pykalmanfilter.hpp"
#include "xo/pyreactor/pyreactor.hpp"
#include "xo/pyutil/pyutil.hpp"

#include "xo/kalmanfilter/init_filter.hpp"
#include "xo/refcnt/Refcounted.hpp"
#include "xo/kalmanfilter/KalmanFilterSvc.hpp"
#include "xo/kalmanfilter/KalmanFilter.hpp"
#include "xo/kalmanfilter/KalmanFilterEngine.hpp"
#include "xo/kalmanfilter/KalmanFilterSpec.hpp"
#include "xo/kalmanfilter/KalmanFilterStep.hpp"
#include "xo/kalmanfilter/KalmanFilterStateToConsole.hpp"
#include "xo/kalmanfilter/KalmanFilterState.hpp"
#include "xo/kalmanfilter/KalmanFilterTransition.hpp"
#include "xo/kalmanfilter/KalmanFilterObservable.hpp"
#include "xo/kalmanfilter/KalmanFilterInputToConsole.hpp"
#include "xo/kalmanfilter/KalmanFilterInput.hpp"
#include "xo/reactor/EventStore.hpp"
#include "xo/subsys/Subsystem.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
//#include <pybind11/operators.h>

namespace xo {
    using xo::kalman::KalmanFilterSvc;
    using xo::kalman::KalmanFilter;
    using xo::kalman::KalmanFilterState;
    using xo::kalman::KalmanFilterEngine;
    using xo::kalman::KalmanFilterSpec;
    using xo::kalman::KalmanFilterStepBase;
    using xo::kalman::KalmanFilterStep;
    using xo::kalman::KalmanFilterStateToConsole;
    using xo::kalman::KalmanFilterState;
    using xo::kalman::KalmanFilterStateExt;
    using xo::kalman::KalmanFilterTransition;
    using xo::kalman::KalmanFilterObservable;
    using xo::kalman::KalmanFilterInputToConsole;
    using xo::kalman::KalmanFilterInput;
    using xo::reactor::AbstractSource;
    using xo::reactor::AbstractSink;
    using xo::reactor::AbstractEventStore;
    using xo::reactor::ReactorSource;
    using xo::reactor::PtrEventStore;
    using xo::reflect::SelfTagging;
    using xo::time::utc_nanos;
    using Eigen::VectorXd;
    using Eigen::VectorXi;
    using Eigen::MatrixXd;
    namespace py = pybind11;

    namespace filter {
        PYBIND11_MODULE(PYKALMANFILTER_MODULE_NAME(), m) {
            /* ensure filter/ will be initialized */
            InitSubsys<S_kalmanfilter_tag>::require();
            /* ..and immediately perform init steps
             *   (this 2nd step is sketchy -- want to pass an application context
             *    to initialize_all())
             */
            Subsystem::initialize_all();

            /* e.g. need python wrapper for for xo::reactor::AbstractSource
             */
            PYREACTOR_IMPORT_MODULE();

            m.doc() = "pybind11 plugin for xo.filter";

            m.def("print_matrix",
                  [](MatrixXd const & m) {
                      std::cout << m << std::endl;
                  });

            m.def("print_vector",
                  [](VectorXd const & v) {
                      std::cout << v << std::endl;
                  });

            // ----- xo::kalman::KalmanFilterState -----

            py::class_<KalmanFilterState,
                       SelfTagging,
                       rp<KalmanFilterState>>(m, "KalmanFilterState")
                .def_static("make",
                            py::overload_cast<uint32_t, utc_nanos, VectorXd, MatrixXd, KalmanFilterTransition>(&KalmanFilterState::make),
                            py::arg("k"), py::arg("tk"),
                            py::arg("x"), py::arg("P"),
                            py::arg("transition"))
                .def("step_no", &KalmanFilterState::step_no)
                .def("tm", &KalmanFilterState::tm)
                .def("n_state", &KalmanFilterState::n_state)
                .def("state_v", &KalmanFilterState::state_v)
                .def("state_cov", &KalmanFilterState::state_cov)
                .def_property_readonly("k", &KalmanFilterState::step_no)
                .def_property_readonly("tk", &KalmanFilterState::tm)
                .def_property_readonly("x", &KalmanFilterState::state_v)
                .def_property_readonly("P", &KalmanFilterState::state_cov)
                .def("__repr__", &KalmanFilterState::display_string);

            // ----- xo::kalman::KalmanFilterStateExt -----

            py::class_<KalmanFilterStateExt,
                       KalmanFilterState,
                       rp<KalmanFilterStateExt>>(m, "KalmanFilterStateExt")
                .def_static("make",
                            py::overload_cast<uint32_t, utc_nanos, VectorXd, MatrixXd, KalmanFilterTransition, MatrixXd, int32_t, rp<KalmanFilterInput>>(&KalmanFilterStateExt::make),
                            py::arg("k"),
                            py::arg("tk"),
                            py::arg("x"),
                            py::arg("P"),
                            py::arg("transition"),
                            py::arg("K"),
                            py::arg("j"),
                            py::arg("zk"))
                .def_property_readonly("j", &KalmanFilterStateExt::observable)
                .def_property_readonly("K", &KalmanFilterStateExt::gain)
                .def_property_readonly("zk", &KalmanFilterStateExt::zk);

            // ----- xo::kalman::KalmanFilterTransition -----

            py::class_<KalmanFilterTransition>(m, "KalmanFilterTransition")
                .def(py::init<MatrixXd, MatrixXd>())
                .def("n_state", &KalmanFilterTransition::n_state)
                .def("transition_mat", &KalmanFilterTransition::transition_mat)
                .def("transition_cov", &KalmanFilterTransition::transition_cov)
                .def("check_ok", &KalmanFilterTransition::check_ok)
                .def_property_readonly("F", &KalmanFilterTransition::transition_mat)
                .def_property_readonly("Q", &KalmanFilterTransition::transition_cov)
                .def("__repr__", &KalmanFilterTransition::display_string);

            // ----- xo::kalman::KalmanFilterObservable -----

            py::class_<KalmanFilterObservable>(m, "KalmanFilterObservable")
                .def(py::init<VectorXi, MatrixXd, MatrixXd>(),
                     py::arg("keep"), py::arg("H"), py::arg("R"))
                .def("n_state", &KalmanFilterObservable::n_state)
                .def("n_observable", &KalmanFilterObservable::n_observable)
                .def("observable_mat", &KalmanFilterObservable::observable)
                .def("observable_cov", &KalmanFilterObservable::observable_cov)
                .def_property_readonly("H", &KalmanFilterObservable::observable)
                .def_property_readonly("R", &KalmanFilterObservable::observable_cov)
                .def("__repr__", &KalmanFilterObservable::display_string);

            // ----- xo::kalman::KalmanFilterInput -----

            py::class_<KalmanFilterInput,
                       SelfTagging,
                       rp<KalmanFilterInput>>(m, "KalmanFilterInput")
                //.def(py::init<utc_nanos, VectorXd>(),
                //     py::arg("tkp1"), py::arg("z"))
                .def("n_observable", &KalmanFilterInput::n_obs)
                .def_property_readonly("tkp1", &KalmanFilterInput::tkp1)
                .def_property_readonly("z", &KalmanFilterInput::z)
                .def("__repr__", &KalmanFilterInput::display_string);

            m.def("make_kalman_filter_input", &KalmanFilterInput::make,
                  py::arg("tkp1"), py::arg("presence"), py::arg("z"), py::arg("zerr"));

            // ----- xo::kalman::KalmanFilterStep -----

            py::class_<KalmanFilterStep>(m, "KalmanFilterStep")
                .def(py::init<rp<KalmanFilterState>, KalmanFilterTransition, KalmanFilterObservable, rp<KalmanFilterInput>>(),
                     py::arg("state"), py::arg("model"), py::arg("obs"), py::arg("input"))
                .def_property_readonly("state", &KalmanFilterStep::state)
                .def_property_readonly("model", &KalmanFilterStepBase::model)
                .def_property_readonly("obs", &KalmanFilterStepBase::obs)
                .def_property_readonly("input", &KalmanFilterStep::input)
                .def("extrapolate", &KalmanFilterStep::extrapolate)
                .def("gain", &KalmanFilterStep::gain)
                .def("gain1", &KalmanFilterStep::gain1)
                .def("correct", &KalmanFilterStep::correct)
                .def("correct1", &KalmanFilterStep::correct1)
                .def("__repr__", &KalmanFilterStep::display_string);

            // ----- xo::kalman::KalmanFilterSpec -----

            py::class_<KalmanFilterSpec>(m, "KalmanFilterSpec")
                .def(py::init<rp<KalmanFilterStateExt>, KalmanFilterSpec::MkStepFn>(),
                     py::arg("s0"), py::arg("mkstepfn"))
                .def("start_ext", &KalmanFilterSpec::start_ext)
                .def("make_step", &KalmanFilterSpec::make_step,
                     py::arg("sk"), py::arg("zkp1"))
                .def("__repr__", &KalmanFilterSpec::display_string);

            // ----- xo::kalman::KalmanFilterEngine -----

            m.def("kf_engine_extrapolate",
                  &KalmanFilterEngine::extrapolate);
            m.def("kf_engine_gain",
                  &KalmanFilterEngine::kalman_gain);
            m.def("kf_engine_gain1",
                  &KalmanFilterEngine::kalman_gain1);
            m.def("kf_engine_correct",
                  &KalmanFilterEngine::correct);
            m.def("kf_engine_correct1",
                  &KalmanFilterEngine::correct1);

            // ----- xo::kalman::KalmanFilter -----

            py::class_<KalmanFilter>(m, "KalmanFilter")
                .def(py::init<KalmanFilterSpec>(),
                     py::arg("spec"))
                .def_property_readonly("step_no", &KalmanFilter::step_no)
                .def_property_readonly("tm", &KalmanFilter::tm)
                .def_property_readonly("filter_spec", &KalmanFilter::filter_spec)
                .def_property_readonly("step", &KalmanFilter::step)
                .def_property_readonly("state_ext", &KalmanFilter::state_ext)
                .def("notify_input", &KalmanFilter::notify_input)
                .def("__repr__", &KalmanFilter::display_string);

            // ----- xo::kalman::KalmanFilterSvc -----

            py::class_<KalmanFilterSvc,
                       ReactorSource,
                       AbstractSink,
                       rp<KalmanFilterSvc>>(m, "KalmanFilterSvc")
                .def_property_readonly("filter", &KalmanFilterSvc::filter)
                .def_property_readonly("last_annexed_ev", &KalmanFilterSvc::last_annexed_ev);

            m.def("make_kalman_filter",
                  &KalmanFilterSvc::make,
                  py::arg("spec"));

            // ----- xo::kalman::KalmanFilterInputToConsole -----

            py::class_<KalmanFilterInputToConsole, AbstractSink, rp<KalmanFilterInputToConsole>>
                (m, "KalmanFilterInputToConsole");

            /* prints KalmanFilterInput events to console */
            m.def("make_kalman_filter_input_printer", &KalmanFilterInputToConsole::make);

            // ----- xo::kalman::KalmanFilterStateToConsole -----

            py::class_<KalmanFilterStateToConsole, AbstractSink, rp<KalmanFilterStateToConsole>>
                (m, "KalmanFilterStateToConsole");

            /* prints KalmanFilterStateExt events to console */
            m.def("make_kalman_filter_state_printer", &KalmanFilterStateToConsole::make);

            // ----- xo::kalman::KalmanFilterStateStore -----

            using KalmanFilterStateEventStore
                = PtrEventStore<rp<KalmanFilterStateExt>>;

            /* see also: UpxEventStore in [pyprocess/pyprocess.cpp]
             *           BboTickStore in [pyoption/pyoption.cpp]
             */
            py::class_<KalmanFilterStateEventStore,
                       AbstractSink,
                       AbstractEventStore,
                       rp<KalmanFilterStateEventStore>>
                (m, "KalmanFilterStateEventStore")
                .def_static("make", &KalmanFilterStateEventStore::make)
                .def("last_n", &KalmanFilterStateEventStore::last_n, py::arg("n"))
                .def("last_dt", &KalmanFilterStateEventStore::last_dt, py::arg("dt"));
            //.def("__repr__", &KalmanFilterStateEventStore::display_string);


#ifdef OBSOLETE
            // ----- xo::option::Pxtick -----

            py::enum_<Pxtick>(m, "Pxtick")
                .value("nickel_dime", Pxtick::nickel_dime);
            //.export_values(); // only need this for pre-c++11-style enum inside a class

            // ----- xo::option::OptionStrikeSet -----

            py::class_<OptionStrikeSet,
                       rp<OptionStrikeSet>>(m, "OptionStrikeSet")
                .def("get_options",
                     [](OptionStrikeSet const & x) {
                         std::vector<rp<VanillaOption>> v;
                         x.append_options(&v);
                         return v;
                     })
#endif
                } /*filter_py*/
    } /*namespace filter*/
} /*namespace xo*/

/* end pykalmanfilter.cpp */
