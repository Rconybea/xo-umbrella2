/* @file pyprintjson.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyprintjson.hpp"
#include <xo/pyreflect/pyreflect.hpp>
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/reflect/TaggedRcptr.hpp>
#include <xo/pyarena/CollectPools.hpp>
#include <xo/pyutil/pyutil.hpp>
#include <pybind11/stl.h>   /* std::vector<MemorySizeInfo>, for visit_pools */

namespace xo {
    namespace py = pybind11;

    namespace json {
        using xo::reflect::SelfTagging;
        using xo::reflect::TaggedRcptr;
        using xo::ref::unowned_ptr;

        namespace {
            /** Enforce at most one PrintJsonAppcx per python instance.
             *  Desirable because context adopts a global singleton
             *  (PrintJsonSingleton::instance()).
             *
             *  @return printjson appcx, to be owned by python.
             **/
            std::unique_ptr<PrintJsonAppcx>
            configure_once(const PrintJsonConfig & cfg,
                           const ReflectAppcx & reflect_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                /* throws rather than silently ignoring cfg: the printer table
                 * is process-wide, so a second configure() could not honour a
                 * different config
                 */
                if (s_configured) {
                    throw std::runtime_error
                        ("xo.printjson.configure: already configured;"
                         " the json printer table is process-wide");
                }

                auto retval = std::make_unique<PrintJsonAppcx>(cfg, reflect_appcx);

                s_configured = true;

                return retval;
            }

            /* no configure_all() here: that convenience belongs at the top
             * of the python stack, not on each subsystem.
             */
        } /*namespace*/

        PYBIND11_MODULE(XO_PYPRINTJSON_MODULE_NAME(), m) {
            /* ReflectAppcx (configure()'s second argument) is registered by
             * xo_pyreflect, and pybind11 permits exactly one registration per
             * c++ type, so we import rather than re-register: without this
             * configure()'s signature does not resolve.
             */
            XO_PYREFLECT_IMPORT_MODULE();

            py::class_<PrintJson, rp<PrintJson>>(m, "PrintJson")
                .def_static("instance", &PrintJsonSingleton::instance)
                .def("print",
                     [](PrintJson & pj, TaggedRcptr p)
                         {
                             pj.print_tp(p, &std::cout); std::cout << "\n";
                         },
                     py::arg("value"))
                .def("print",
                     [](PrintJson & pj, rp<SelfTagging> const & p)
                         {
                             pj.print_obj(p, &std::cout); std::cout << "\n";
                         },
                     py::arg("value"));

            //m.def("print_json", [](){ return PrintJsonSingleton::instance_ptr(); });

            // ----------------------------------------------------------------
            // subsystem configuration and context.
            //
            // Import registers types and nothing else.  A context is built only
            // by configure(), so python supplies configuration the same way a
            // c++ main() does.

            py::class_<PrintJsonConfig>(m, "PrintJsonConfig")
                .def(py::init<>(),
                     "configuration for the xo-printjson subsystem"
                     " (no settings yet)")
                .def("__repr__", [](const PrintJsonConfig &) {
                        return std::string("<PrintJsonConfig>"); });

            py::class_<PrintJsonAppcx>(m, "PrintJsonAppcx")
                /* what this context was actually configured with.  Without it
                 * configuration is write-only from python: a caller cannot
                 * confirm the values it passed took effect, and a test cannot
                 * assert it.
                 */
                .def("config", &PrintJsonAppcx::config,
                     py::return_value_policy::reference_internal,
                     "the PrintJsonConfig this context was established with")
                /* the printer table, reached through the context rather than
                 * through PrintJson.instance().  Same object today -- the
                 * context adopts the singleton -- but reaching it this way is
                 * what lets the singleton be retired later without changing
                 * callers (.xo-backlog/xo-printjson/issues/03).
                 */
                .def("print_json", &PrintJsonAppcx::print_json,
                     "the PrintJson this context established")
                /* Placeholder, and says so: xo-printjson has no pools yet, so
                 * this is always empty.  Bound now so the shape matches every
                 * other Appcx -- code walking the stack need not special-case
                 * this one.
                 */
                .def("visit_pools", &xo::pyarena::collect_pools<PrintJsonAppcx>,
                     "xo-printjson memory pools, as a list of MemorySizeInfo."
                     "  Always empty until PrintJson uses a DArena")
                .def("__repr__", [](const PrintJsonAppcx &) {
                        return std::string("<PrintJsonAppcx>"); });

            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("reflect_appcx"),
                  py::keep_alive<0, 2>(),
                  "establish an xo-printjson context, and return it."
                  "  The caller owns it; when the last python reference goes,"
                  " so does the context."
                  "  Takes the context returned by xo.reflect.configure(),"
                  " which xo-printjson is built on."
                  "  Throws if already configured: the json printer table is"
                  " process-wide, so a second context could not honour a"
                  " different config.");
        } /*pyprintjson*/
    } /*namespace json*/
} /*namespace xo*/

/* end pyprintjson.cpp */
