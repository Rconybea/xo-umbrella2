/* @file pyfacet.cpp
 *
 * python bindings for xo-facet (the faceted object model).
 */

#include "pyfacet.hpp"
#include <xo/pyarena/pyarena.hpp>
#include <xo/pyarena/CollectPools.hpp>
#include <xo/pyindentlog2/pyindentlog2.hpp>
#include <xo/facet/AllocFlywheel.hpp>
#include <xo/facet/cx/FacetAppcx.hpp>
#include <xo/facet/cx/FacetConfig.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/PrettyVector.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <pybind11/stl.h>   /* std::optional <-> None */
#include <memory>
#include <optional>
#include <stdexcept>

namespace xo {
    namespace py = pybind11;

    using xo::mm::AllocFlywheel;
    using xo::pyarena::collect_pools;
    using xo::mm::ArenaConfig;
    using xo::pp::TempPpSink;
    using xo::pp::PpSink;

    namespace facet {
        namespace {
            /** the FacetAppcx for this python process.
             *
             *  Owned here rather than in xo-facet, so that a standalone c++
             *  application never acquires a singleton.  See the equivalent in
             *  xo-pyindentlog2 for why an import cannot be the trigger.
             **/
            struct PyFacetAppcx {
                static std::unique_ptr<FacetAppcx> appcx_;

                static FacetAppcx & appcx() {
                    if (!appcx_) {
                        throw std::runtime_error
                            ("xo_pyfacet.appcx: not configured;"
                             " call xo_pyfacet.configure(cfg) first");
                    }

                    return *appcx_;
                }

                /** @p il_appcx  the context this one is built on, as
                 *  returned by xo_pyindentlog2.configure().
                 *
                 *  Taken as an argument rather than fetched from that module:
                 *  xo-facet's dependence on xo-indentlog2 is then visible in
                 *  the call, and mirrors the c++ ctor, which asks for the same
                 *  thing (FacetAppcx(cfg, const Indentlog2Appcx &)).
                 *
                 *  @return the context just established.
                 **/
                static FacetAppcx & configure(const FacetConfig & cfg,
                                              const Indentlog2Appcx & il_appcx)
                {
                    if (appcx_) {
                        throw std::runtime_error
                            ("xo_pyfacet.configure: already configured;"
                             " capacities cannot be changed after the first call");
                    }

                    appcx_ = std::make_unique<FacetAppcx>(cfg, il_appcx);

                    return *appcx_;
                }
            };

            std::unique_ptr<FacetAppcx> PyFacetAppcx::appcx_;

            /** configure xo-facet and everything below it, in one call.
             *
             *  Convenience over the explicit chain
             *
             *    il_cx = xo_pyindentlog2.configure(il_cfg)
             *    f_cx  = xo_pyfacet.configure(f_cfg, il_cx)
             *
             *  for the common case where a caller wants the whole stack and
             *  has no use for the intermediate context.
             *
             *  This reaches into xo_pyindentlog2, but not silently: taking an
             *  Indentlog2Config is what says so.  The argument is the evidence
             *  -- which is the difference between a convenience and a secret.
             *
             *  Strict, like configure(): throws if any level is already
             *  configured, rather than quietly ignoring a config the caller
             *  asked to take effect.
             *
             *  @return this module's context (xo_pyindentlog2.appcx() has the
             *  other one, if it is wanted later).
             **/
            /** @p f_cfg, @p il_cfg  omitted (or None) means "no opinion" --
             *  the subsystem's own make_default() is used.
             *
             *  Defaults are materialized HERE, per call, rather than as pybind
             *  default arguments: those are evaluated once when the module is
             *  imported, and an import should do nothing but register types.
             *  Constructing a config is inert today (appcx-config/02 moved the
             *  InitEvidence out of configs), but pinning the default at import
             *  time would quietly depend on it staying that way.
             **/
            FacetAppcx & configure_all(std::optional<FacetConfig> f_cfg,
                                       std::optional<Indentlog2Config> il_cfg)
            {
                /* through the module object, not by linkage: xo_pyindentlog2
                 * owns its own context, and python loads modules RTLD_LOCAL
                 */
                auto il_module = py::module_::import(PYINDENTLOG2_MODULE_NAME_STR);

                auto & il_appcx
                    = il_module.attr("configure")
                          (il_cfg.value_or(Indentlog2Config::make_default()))
                          .cast<Indentlog2Appcx &>();

                return PyFacetAppcx::configure(f_cfg.value_or(FacetConfig::make_default()),
                                               il_appcx);
            }
        } /*namespace*/

        PYBIND11_MODULE(PYFACET_MODULE_NAME(), m) {
            /* ArenaConfig (make_app's arguments) and PpSink (pretty's) are
             * registered by these modules, and pybind11 permits exactly one
             * registration per c++ type.  Imported separately rather than
             * relying on xo_pyindentlog2 to pull xo_pyarena in transitively:
             * this module names ArenaConfig itself.
             */
            PYARENA_IMPORT_MODULE();
            PYINDENTLOG2_IMPORT_MODULE();

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.facet";

            // ----------------------------------------------------------------
            // FacetAppcx

            // ----------------------------------------------------------------
            // subsystem configuration and context.  Import registers types;
            // configure() builds the context, from python-supplied capacities.

            py::class_<FacetConfig>(m, "FacetConfig")
                .def(py::init<std::uint32_t, std::uint32_t>(),
                     py::arg("facet_registry_capacity"),
                     py::arg("type_registry_capacity"),
                     "configuration for the xo-facet subsystem")
                /* the defaults live in c++ (FacetConfig::make_default), so python and
                 * a c++ main() get the same ones -- rather than this binding
                 * inventing a second set that could drift
                 */
                .def_static("make_default", &FacetConfig::make_default,
                            "default configuration")
                .def("facet_registry_capacity", &FacetConfig::facet_registry_capacity)
                .def("type_registry_capacity", &FacetConfig::type_registry_capacity)
                .def("__repr__",
                     [](const FacetConfig & x) {
                         return ("<FacetConfig facet_registry_capacity="
                                 + std::to_string(x.facet_registry_capacity())
                                 + " type_registry_capacity="
                                 + std::to_string(x.type_registry_capacity()) + ">");
                     });

            /* opaque: python holds it only to hand to the module above */
            py::class_<FacetAppcx>(m, "FacetAppcx")
                /* see the note on Indentlog2Appcx.config: makes configuration
                 * checkable rather than write-only
                 */
                .def("config", &FacetAppcx::config,
                     py::return_value_policy::reference_internal,
                     "the FacetConfig this context was established with")
                /* the context this one was built on -- the witness chain,
                 * navigable from python as it is from c++
                 */
                .def("indentlog2_appcx", &FacetAppcx::indentlog2_appcx,
                     py::return_value_policy::reference_internal,
                     "the xo-indentlog2 context this one stands on")
                /* memory reporting.  Returns the pools rather than taking a
                 * visitor: the snapshots have to be materialized either way
                 * (see CollectPools.hpp), so a list is the friendlier shape.
                 *
                 * Three pools: the facet registry is a hash map and reports
                 * its control and slot arenas separately ("facets-ctl",
                 * "facets-slots"); the type registry reports one ("types").
                 *
                 * As in c++ this does NOT descend into the
                 * indentlog2 context -- ask that one separately and
                 * concatenate, or a caller walking the chain double-counts.
                 */
                .def("visit_pools", &collect_pools<FacetAppcx>,
                     "this context's memory pools, as a list of MemorySizeInfo."
                     "  Does not include the indentlog2 context's pools")
                .def("__repr__", [](const FacetAppcx &) {
                        return std::string("<FacetAppcx>"); });

            m.def("configure", &PyFacetAppcx::configure,
                  py::arg("config"),
                  py::arg("indentlog2_appcx"),
                  py::return_value_policy::reference,
                  "establish this process's xo-facet context, and return it."
                  "  Takes the context returned by xo_pyindentlog2.configure(),"
                  " which xo-facet is built on."
                  "  Throws if already configured.");

            m.def("configure_all", &configure_all,
                  py::arg("facet_config") = py::none(),
                  py::arg("indentlog2_config") = py::none(),
                  py::return_value_policy::reference,
                  "configure xo-facet and the xo-indentlog2 it stands on,"
                  " and return the xo-facet context."
                  "  Either config may be omitted, meaning that subsystem's"
                  " make_default()."
                  "  Equivalent to configure(facet_config,"
                  " xo_pyindentlog2.configure(indentlog2_config))."
                  "  Throws if either is already configured.");

            m.def("appcx", &PyFacetAppcx::appcx,
                  py::return_value_policy::reference,
                  "this process's xo-facet context."
                  "  Throws if configure() has not been called.");

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

                .def_static("make_app",
                            &AllocFlywheel::make_app,
                            py::arg("appcx"),
                            py::arg("storage_cfg"),
                            py::arg("strong_root_cfg"),
                            py::arg("weak_root_cfg"),
                            "create a flywheel: primary arena, strong root set,"
                            " weak root set")

                .def_static("make_default_app",
                            &AllocFlywheel::make_default_app,
                            py::arg("appcx"),
                            "create a flywheel: with default config for arena storage")

                /* memory reporting.  Returns the pools rather than taking a
                 * visitor, as FacetAppcx.visit_pools() does: the snapshots
                 * have to be materialized either way (see CollectPools.hpp).
                 *
                 * Three pools, in the order the handle store visits them: the
                 * primary arena, then the strong and weak root sets.
                 */
                .def("visit_pools", &collect_pools<AllocFlywheel>,
                     "this flywheel's memory pools, as a list of MemorySizeInfo:"
                     " storage arena, strong root set, weak root set")

                /* renders into a sink supplied by the caller -- typically an
                 * xo_pyindentlog2.PrettySink.  Declared as PpSink & so any
                 * sink works, not just PrettySink.
                 */
                .def("pretty",
                     [](const AllocFlywheel & self, PpSink & sink) {
                         self.pretty(sink);
                     },
                     py::arg("sink"),
                     "pretty-print this flywheel into sink")

                .def("__repr__",
                     [](const AllocFlywheel & self) {
                         static_assert(xo::carries_indentlog2<AllocFlywheel>);

                         return TempPpSink::pp2str(self);
                     });

        } /*pyfacet*/
    } /*namespace facet*/
} /*namespace xo*/

/* end pyfacet.cpp */
