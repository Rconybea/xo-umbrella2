/* @file pyreflect.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyreflect.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include "xo/reflect/TaggedRcptr.hpp"
#include "xo/reflect/SelfTagging.hpp"
//#include "time/Time.hpp"
//#include "xo/pyutil/pytime.hpp"
#include "xo/pyutil/pyutil.hpp"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/chrono.h>
//#include <pybind11/operators.h>

namespace xo {
    using xo::time::utc_nanos;
    using xo::ref::unowned_ptr;
    using xo::ref::rp;
    namespace py = pybind11;

    namespace reflect {
        PYBIND11_MODULE(PYREFLECT_MODULE_NAME(), m) {

            /* note: possibly move this to pytime/  if/when we provide it */
            //py::class_<utc_nanos>(m, "utc_nanos");

            //py::class_<TypeDescrImpl>(m, "TypeDescr");
            py::class_<TypeDescrBase,
                       unowned_ptr<TypeDescrBase>>(m, "TypeDescr")
                .def_static("print_reflected_types",
                            [](){ TypeDescrBase::print_reflected_types(std::cout); })
                .def_property_readonly("canonical_name", &TypeDescrBase::canonical_name)
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

        } /*pyreflect*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end pyreflect.cpp */
