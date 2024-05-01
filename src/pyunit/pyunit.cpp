/* @file pyunit.cpp */

// note: need pyreflect/ here bc pyreflect.hpp is generated, located in build directory
#include "pyunit.hpp"
#include "xo/unit/xquantity.hpp"
#include "xo/unit/xquantity_iostream.hpp"
#include "xo/pyutil/pyutil.hpp"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/chrono.h>
//#include <pybind11/operators.h>

namespace xo {
    namespace py = pybind11;
    using Unit = xo::qty::natural_unit<std::int64_t>;
    using XoQuantity = xo::qty::Quantity<double, std::int64_t>;

    namespace qty {
        PYBIND11_MODULE(PYUNIT_MODULE_NAME(), m) {

            m.doc() = "pybind11 plugin for xo.unit";

            py::class_<Unit>(m, "Unit")
                .def("__repr__",
                     [](Unit & x)
                         {
                             /* e.g. "1g" for xo::qty::nu::gram */
                             return tostr(1, x.abbrev());
                         })
                ;

            py::module unit = m.def_submodule("unit");

            unit.attr("microgram") = &xo::qty::nu::microgram;
            unit.attr("milligram") = &xo::qty::nu::milligram;
            unit.attr("gram") = &xo::qty::nu::gram;
            unit.attr("kilogram") = &xo::qty::nu::kilogram;
            unit.attr("tonne") = &xo::qty::nu::tonne;

            py::module qty = m.def_submodule("qty");

            qty.def("micrograms", [](double x) { return XoQuantity(x, nu::microgram); });
            qty.def("milligrams", [](double x) { return XoQuantity(x, nu::milligram); });
            qty.def("grams", [](double x) { return XoQuantity(x, nu::gram); });
            qty.def("kilograms", [](double x) { return XoQuantity(x, nu::kilogram); });
            qty.def("tonnes", [](double x) { return XoQuantity(x, nu::tonne); });

            py::class_<XoQuantity>(m, "Quantity")
                .def(py::init<double, const Unit &>(),
                     py::arg("scale"), py::arg("unit"))
                .def("__mul__",
                     [](const XoQuantity & x, const XoQuantity & y)
                         {
                             return x * y;
                         })
                .def("__mul__",
                     [](const XoQuantity & x, double y)
                         {
                             return x * y;
                         })
                .def("__rmul__",
                     [](const XoQuantity & y, double x)
                         {
                             return x * y;
                         })

                .def("__truediv__",
                     [](const XoQuantity & x, const XoQuantity & y)
                         {
                             return x / y;
                         })
                .def("__truediv__",
                     [](const XoQuantity & x, double y)
                         {
                             return x / y;
                         })
                .def("__rtruediv__",
                     [](const XoQuantity & y, double x)
                         {
                             return x / y;
                         })

                .def("__repr__",
                     [](const XoQuantity & x)
                         {
                             return tostr(x);
                         })
                ;
        }
    }
} /*namespace xo*/
