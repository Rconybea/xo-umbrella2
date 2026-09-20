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
            /** Enforce at most one FacetAppcx per python instance.
             *  Desirable because context sets up global singletons
             *  (FacetRegistry, TypeRegistry).
             *
             *  @p il_appcx  Context for xo-indentlog2.
             *  (see xo-pyindentlog2.configure()).
             *
             *  @return facet appcx, to be owned by python.
             **/
            std::unique_ptr<FacetAppcx>
            configure_once(const FacetConfig & cfg,
                           const Indentlog2Appcx & il_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                /* throws rather than silently ignoring cfg: capacities are
                 * honored on first construction only, so a second
                 * configure() could not deliver what it appears to promise
                 */
                if (s_configured) {
                    throw std::runtime_error
                        ("xo_pyfacet.configure: already configured;"
                         " capacities cannot be changed after the first call");
                }

                auto retval = std::make_unique<FacetAppcx>(cfg, il_appcx);

                s_configured = true;

                return retval;
            }

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
            py::object configure_all(std::optional<FacetConfig> f_cfg,
                                     std::optional<Indentlog2Config> il_cfg)
            {
                /* through the module object, not by linkage: xo_pyindentlog2
                 * owns its own context, and python loads modules RTLD_LOCAL
                 */
                auto il_module = py::module_::import(XO_PYINDENTLOG2_MODULE_NAME_STR);

                /* HELD, not cast from a temporary.  xo_pyindentlog2.configure()
                 * now hands ownership to its caller, so the returned python
                 * object IS the context's owner: casting a temporary to
                 * Indentlog2Appcx & would destroy it at the end of the full
                 * expression and leave the FacetAppcx below holding a dangling
                 * reference.
                 */
                py::object il_obj
                    = il_module.attr("configure")
                          (il_cfg.value_or(Indentlog2Config::make_default()));

                auto & il_appcx = il_obj.cast<Indentlog2Appcx &>();

                py::object f_obj
                    = py::cast(configure_once(f_cfg.value_or(FacetConfig::make_default()),
                                              il_appcx));

                /* the same edge py::keep_alive<0,2> declares on configure(),
                 * established by hand because here the patient is a local
                 * rather than an argument: the returned context holds a
                 * reference into il_obj, so il_obj must outlive it.  Without
                 * this the indentlog2 context dies when this function returns.
                 */
                py::detail::keep_alive_impl(f_obj, il_obj);

                return f_obj;
            }
        } /*namespace*/

        PYBIND11_MODULE(XO_PYFACET_MODULE_NAME(), m) {
            /* ArenaConfig (make_app's arguments) and PpSink (pretty's) are
             * registered by these modules, and pybind11 permits exactly one
             * registration per c++ type.  Imported separately rather than
             * relying on xo_pyindentlog2 to pull xo_pyarena in transitively:
             * this module names ArenaConfig itself.
             */
            XO_PYARENA_IMPORT_MODULE();
            XO_PYINDENTLOG2_IMPORT_MODULE();

            /* module docstring */
            m.doc() = "pybind11 plugin for xo.facet";

            // ----------------------------------------------------------------
            // FacetAppcx

            // ----------------------------------------------------------------
            // subsystem configuration and context.  Import registers types;
            // configure() builds the context, from python-supplied capacities.

            py::class_<FacetConfig>(m, "FacetConfig")
                .def(py::init<std::uint32_t, std::uint32_t, std::uint32_t>(),
                     py::arg("facet_registry_capacity"),
                     py::arg("type_registry_capacity"),
                     py::arg("storage_base_align"),
                     "configuration for the xo-facet subsystem")
                /* the defaults live in c++ (FacetConfig::make_default), so python and
                 * a c++ main() get the same ones -- rather than this binding
                 * inventing a second set that could drift
                 */
                .def_static("make_default", &FacetConfig::make_default,
                            "default configuration")
                .def("facet_registry_capacity", &FacetConfig::facet_registry_capacity)
                .def("type_registry_capacity", &FacetConfig::type_registry_capacity)
                .def("storage_base_align", &FacetConfig::storage_base_align)
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

            /* keep_alive<0,2>: the returned context holds a reference to the
             * indentlog2 context (argument 2), so python must not collect that
             * one first.  This is what makes the stack safe to build a level at
             * a time -- the dependency edge becomes a python reference.
             */
            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("indentlog2_appcx"),
                  py::keep_alive<0, 2>(),
                  "establish an xo-facet context, and return it."
                  "  The caller owns it; when the last python reference goes,"
                  " so does the context."
                  "  Takes the context returned by xo_pyindentlog2.configure(),"
                  " which xo-facet is built on."
                  "  Throws if already configured: FacetRegistry and"
                  " TypeRegistry are process-wide, so a second context could"
                  " not honour a different config.");

            m.def("configure_all", &configure_all,
                  py::arg("facet_config") = py::none(),
                  py::arg("indentlog2_config") = py::none(),
                  "configure xo-facet and the xo-indentlog2 it stands on,"
                  " and return the xo-facet context."
                  "  Either config may be omitted, meaning that subsystem's"
                  " make_default()."
                  "  Equivalent to configure(facet_config,"
                  " xo_pyindentlog2.configure(indentlog2_config))."
                  "  Throws if either is already configured.");

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

                /* keep_alive<0,1>: the flywheel must not outlive the context
                 * it was made from.
                 */
                .def_static("make_app",
                            &AllocFlywheel::make_app,
                            py::arg("appcx"),
                            py::arg("storage_cfg"),
                            py::arg("strong_root_cfg"),
                            py::keep_alive<0, 1>(),
                            "create a flywheel: primary arena, strong root set")

                .def_static("make_default_app",
                            &AllocFlywheel::make_default_app,
                            py::arg("appcx"),
                            py::keep_alive<0, 1>(),
                            "create a flywheel: with default config for arena storage")

                /* memory reporting.  Returns owne memory pools. */
                .def("visit_pools", &collect_pools<AllocFlywheel>,
                     "this flywheel's memory pools, as a list of MemorySizeInfo:"
                     " storage arena, strong root set, and the free list"
                     " serving it")

                /* how many in-use roots are currently held.
                 */
                .def("strong_root_count", &AllocFlywheel::strong_root_count,
                     "occupied strong slots.  Released slots are deducted, so"
                     " this falls when a handle is dropped -- it is not the"
                     " high-water mark")

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
                         static_assert(xo::carries_indentlog2_appcx<AllocFlywheel>);

                         return TempPpSink::pp2str(self);
                     });

        } /*pyfacet*/
    } /*namespace facet*/
} /*namespace xo*/

/* end pyfacet.cpp */
