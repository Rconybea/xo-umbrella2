/* @file pyprintjson.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyprintjson.hpp"
#include "xo/pyreflect/pyreflect.hpp"

#include "xo/printjson/PrintJson.hpp"
#include "xo/reflect/TaggedRcptr.hpp"
//#include "reflect/SelfTagging.hpp"
//#include "refcnt/Refcounted.hpp"
//#include "refcnt/Unowned.hpp"
#include "xo/pyutil/pyutil.hpp"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/chrono.h>
//#include <pybind11/operators.h>

namespace xo {
    namespace py = pybind11;

    namespace json {
        using xo::reflect::SelfTagging;
        using xo::reflect::TaggedRcptr;
        using xo::ref::unowned_ptr;

        PYBIND11_MODULE(PYPRINTJSON_MODULE_NAME(), m) {
            PYREFLECT_IMPORT_MODULE();

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
        } /*pyprintjson*/
    } /*namespace json*/
} /*namespace xo*/

/* end pyprintjson.cpp */
