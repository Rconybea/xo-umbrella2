/* @file pyunit.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyunit.hpp"
#include "xo/unit/xquantity.hpp"
#include "xo/pyutil/pyutil.hpp"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/chrono.h>
//#include <pybind11/operators.h>

namespace xo {
    namespace py = pybind11;

    namespace qty {
        PYBIND11_MODULE(PYUNIT_MODULE_NAME(), m) {

            py::class_<Quantity>(m, "Quantity")
                .def("__repr__",
                     [](Quantity & x)
                         {
                             return tostr(x);
                         })
                py::arg("qty"));
        }
    }
} /*namespace xo*/
