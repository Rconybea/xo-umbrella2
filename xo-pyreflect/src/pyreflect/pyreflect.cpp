/* @file pyreflect.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyreflect.hpp"
#include <xo/reflect/cx/ReflectAppcx.hpp>
#include <xo/reflect/SelfTagging.hpp>
#include <xo/reflect/TaggedRcptr.hpp>
#include <xo/reflect/TypeDescr.hpp>
#include <xo/timeutil/timeutil.hpp>
// #include "time/Time.hpp"
// #include "xo/pyutil/pytime.hpp"
#include <xo/pyindentlog2/pyindentlog2.hpp>
#include <xo/pyarena/CollectPools.hpp>
#include <xo/pyutil/pyutil.hpp>
//#include <pybind11/pybind11.h>
#include <pybind11/stl.h>   /* std::vector<MemorySizeInfo>, for visit_pools */
//#include <pybind11/chrono.h>
//#include <pybind11/operators.h>
#include <iostream>

namespace xo {
    using xo::time::utc_nanos;
    using xo::ref::unowned_ptr;
    using xo::pp::FlatSink;
    using xo::pp::PpStyle;
    using xo::rp;
    namespace py = pybind11;

    namespace reflect {
        namespace {
            /** Enforce at most one ReflectAppcx per python instance.
             *  Desirable because context adopts a global singleton
             *  (TypeDescrTable::instance()).
             *
             *  @return reflect appcx, to be owned by python.
             **/
            std::unique_ptr<ReflectAppcx>
            configure_once(const ReflectConfig & cfg,
                           const Indentlog2Appcx & il_appcx)
            {
                /** true once this function has run **/
                static bool s_configured = false;

                /* throws rather than silently ignoring cfg: the type table is
                 * process-wide, so a second configure() could not honour a
                 * different config
                 */
                if (s_configured) {
                    throw std::runtime_error
                        ("xo.reflect.configure: already configured;"
                         " TypeDescrTable is process-wide");
                }

                auto retval = std::make_unique<ReflectAppcx>(cfg, il_appcx);

                s_configured = true;

                return retval;
            }

            /* no configure_all() here: that convenience belongs at the top
             * of the python stack, not on each subsystem.
             */
        } /*namespace*/

        PYBIND11_MODULE(XO_PYREFLECT_MODULE_NAME(), m) {

            m.doc() = "pybind11 plugin for xo.reflect";

            /* Indentlog2Appcx (configure()'s second argument) is registered by
             * xo_pyindentlog2, and pybind11 permits exactly one registration
             * per c++ type, so we import rather than re-register: without this
             * configure()'s signature does not resolve.
             */
            XO_PYINDENTLOG2_IMPORT_MODULE();

            py::enum_<Metatype>(m, "Metatype")
                .value("invalid", Metatype::mt_invalid)
                .value("atomic", Metatype::mt_atomic)
                .value("pointer", Metatype::mt_pointer)
                .value("vector", Metatype::mt_vector)
                .value("struct", Metatype::mt_struct)
                .value("function", Metatype::mt_function)
                ;

            /* note: possibly move this to pytime/  if/when we provide it */
            //py::class_<utc_nanos>(m, "utc_nanos");

            //py::class_<TypeDescrImpl>(m, "TypeDescr");
            /* TypeDescrBase instances are created automatically at library load time
             * by static initializers.  The reflection library (xo-reflect) is responsible
             * for lifetime of TypeDescrobjects.  Under no circumstances should python
             * (or pybind11) directly destroy a TypeDescrImpl instance, hence use of
             * unowned_ptr<TypeDescrBase> here.
             */
            py::class_<TypeDescrBase,
                       unowned_ptr<TypeDescrBase>>(m, "TypeDescr")

                .def_static("lookup_by_name", &TypeDescrBase::lookup_by_name)
                .def_static("print_reflected_types",
                            [](){
                                FlatSink sink(PpStyle::colored(), std::cout.rdbuf());
                                TypeDescrBase::print_reflected_types(sink); })

                .def_property_readonly("canonical_name", &TypeDescrBase::canonical_name)
                .def_property_readonly("short_name", &TypeDescrBase::short_name)
                .def_property_readonly("metatype", &TypeDescrBase::metatype)
                .def_property_readonly("complete_flag", &TypeDescrBase::complete_flag)
                .def("__repr__", &TypeDescrBase::display_string);

            /* note: this means python will use
             *         std::unique_ptr<TaggedRcptr>
             *       when it encounters a TaggedRcptr instance.
             *       Maintains refcount at cost of 2nd level of indirection.
             */
            py::class_<TaggedRcptr>(m, "TaggedRcptr")
                .def_property_readonly("td", &TaggedPtr::td)
                .def("__repr__", &TaggedRcptr::display_string);

            py::class_<SelfTagging,
                       rp<SelfTagging>>(m, "SelfTagging")
                .def("self_tp", &SelfTagging::self_tp);

            // ----------------------------------------------------------------
            // subsystem configuration and context.
            //
            // Import registers types and nothing else.  A context is built only
            // by configure(), so python supplies configuration the same way a
            // c++ main() does.

            py::class_<ReflectConfig>(m, "ReflectConfig")
                .def(py::init<>(),
                     "configuration for the xo-reflect subsystem"
                     " (no settings yet)")
                .def("__repr__", [](const ReflectConfig &) {
                        return std::string("<ReflectConfig>"); });

            py::class_<ReflectAppcx>(m, "ReflectAppcx")
                /* what this context was actually configured with.  Without it
                 * configuration is write-only from python: a caller cannot
                 * confirm the values it passed took effect, and a test cannot
                 * assert it.
                 */
                .def("config", &ReflectAppcx::config,
                     py::return_value_policy::reference_internal,
                     "the ReflectConfig this context was established with")
                /* Placeholder, and says so: xo-reflect has no pools yet, so
                 * this is always empty.  It becomes meaningful once
                 * TypeDescrTable is represented with a DArena.  Bound now so
                 * the shape matches every other Appcx -- code walking the
                 * stack need not special-case this one.
                 */
                .def("visit_pools", &xo::pyarena::collect_pools<ReflectAppcx>,
                     "xo-reflect memory pools, as a list of MemorySizeInfo."
                     "  Always empty until TypeDescrTable uses a DArena")
                .def("__repr__", [](const ReflectAppcx &) {
                        return std::string("<ReflectAppcx>"); });

            m.def("configure", &configure_once,
                  py::arg("config"),
                  py::arg("indentlog2_appcx"),
                  py::keep_alive<0, 2>(),
                  "establish an xo-reflect context, and return it."
                  "  The caller owns it; when the last python reference goes,"
                  " so does the context."
                  "  Takes the context returned by xo.indentlog2.configure(),"
                  " which xo-reflect is built on."
                  "  Throws if already configured: TypeDescrTable is"
                  " process-wide, so a second context could not honour a"
                  " different config.");

        } /*pyreflect*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end pyreflect.cpp */
