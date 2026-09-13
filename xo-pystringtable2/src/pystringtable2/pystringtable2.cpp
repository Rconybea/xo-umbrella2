/* @file pystringtable2.cpp */

// note: need pystringtable2/ here bc pystringtable2.hpp is generated,
//       located in build directory
#include "pystringtable2.hpp"
#include <xo/pyfacet/pyfacet.hpp>
#include <xo/stringtable2/cx/Stringtable2Appcx.hpp>
#include <xo/pyarena/CollectPools.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <pybind11/stl.h>   /* std::vector<MemorySizeInfo>, for visit_pools */

namespace xo {
    namespace py = pybind11;

    namespace scm {
        namespace {
            /** Enforce at most one Stringtable2Appcx per python instance.
                *
             *  @return stringtable2 appcx, to be owned by python.
             **/
            std::unique_ptr<Stringtable2Appcx>
            configure_once(const Stringtable2Config & cfg,
                           const FacetAppcx & facet_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                if (s_configured) {
                    throw std::runtime_error
                        ("xo.stringtable2.configure: already configured;"
                         " facet registration is process-wide");
                }

                auto retval = std::make_unique<Stringtable2Appcx>(cfg, facet_appcx);

                s_configured = true;

                return retval;
            }

            /* no configure_all() here: that convenience belongs at the top
             * of the python stack, not on each subsystem.
             */
        } /*namespace*/

        PYBIND11_MODULE(XO_PYSTRINGTABLE2_MODULE_NAME(), m) {
            /* module docstring */
            m.doc() = "pybind11 plugin for xo.stringtable2";

            /* FacetAppcx (configure()'s second argument) is registered by
             * xo_pyfacet, and pybind11 permits exactly one registration per
             * c++ type, so we import rather than re-register: without this
             * configure()'s signature does not resolve.
             */
            XO_PYFACET_IMPORT_MODULE();

            // ----------------------------------------------------------------
            // subsystem configuration and context.
            //
            // Import registers types and nothing else.  A context is built only
            // by configure(), so python supplies configuration the same way a
            // c++ main() does.

            py::class_<Stringtable2Config>(m, "Stringtable2Config")
                .def(py::init<>(),
                     "configuration for the xo-stringtable2 subsystem."
                     "  Empty, and expected to stay so: a StringTable's"
                     " capacity belongs to whoever constructs one, not to the"
                     " application")
                .def("__repr__", [](const Stringtable2Config &) {
                        return std::string("<Stringtable2Config>"); });

            py::class_<Stringtable2Appcx>(m, "Stringtable2Appcx")
                /* what this context was actually configured with.  Without it
                 * configuration is write-only from python: a caller cannot
                 * confirm the values it passed took effect, and a test cannot
                 * assert it.
                 */
                .def("config", &Stringtable2Appcx::config,
                     py::return_value_policy::reference_internal,
                     "the Stringtable2Config this context was established with")
                /* Empty, and NOT a placeholder -- unlike ReflectAppcx and
                 * PrintJsonAppcx, this one is not waiting on a DArena
                 * refactor.  xo-stringtable2 owns no pool.  A StringTable does
                 * have pools and its own visit_pools, but it belongs to its
                 * constructor's caller, who is the one able to report it.
                 */
                .def("visit_pools", &xo::pyarena::collect_pools<Stringtable2Appcx>,
                     "always empty: xo-stringtable2 owns no memory pool."
                     "  A StringTable reports its own, to whoever owns it")
                .def("__repr__", [](const Stringtable2Appcx &) {
                        return std::string("<Stringtable2Appcx>"); });

            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("facet_appcx"),
                  py::keep_alive<0, 2>(),
                  "establish an xo-stringtable2 context, and return it."
                  "  The caller owns it; when the last python reference goes,"
                  " so does the context."
                  "  Takes the context returned by xo.facet.configure(), which"
                  " owns the FacetRegistry this subsystem registers into."
                  "  Throws if already configured.");
        } /*pystringtable2*/
    } /*namespace scm*/
} /*namespace xo*/

/* end pystringtable2.cpp */
