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
    using XoQuantity = xo::qty::xquantity<double, std::int64_t>;

    namespace qty {
        PYBIND11_MODULE(PYUNIT_MODULE_NAME(), m) {

            m.doc() = "pybind11 plugin for xo.unit";

            py::class_<Unit>(m, "Unit")
                .def("__repr__",
                     [](Unit & x)
                         {
                             /* e.g. "<unit g>" for xo::qty::nu::gram */
                             return tostr("<Unit ", x.abbrev(), ">");
                         })
                ;

            py::module unit = m.def_submodule("unit");

            unit.attr("picogram")  = &xo::qty::nu::picogram;
            unit.attr("nanoogram") = &xo::qty::nu::nanogram;
            unit.attr("microgram") = &xo::qty::nu::microgram;
            unit.attr("milligram") = &xo::qty::nu::milligram;
            unit.attr("gram")      = &xo::qty::nu::gram;
            unit.attr("kilogram")  = &xo::qty::nu::kilogram;
            unit.attr("tonne")     = &xo::qty::nu::tonne;
            unit.attr("kilotonne") = &xo::qty::nu::kilotonne;
            unit.attr("megatonne") = &xo::qty::nu::megatonne;

            unit.attr("picometer") = &xo::qty::nu::picometer;
            unit.attr("nanometer") = &xo::qty::nu::nanometer;
            unit.attr("micrometer") = &xo::qty::nu::micrometer;
            unit.attr("millimeter") = &xo::qty::nu::millimeter;
            unit.attr("meter") = &xo::qty::nu::meter;
            unit.attr("kilometer") = &xo::qty::nu::kilometer;
            unit.attr("megameter") = &xo::qty::nu::megameter;

            py::module qty = m.def_submodule("qty");

            qty.def("picograms",  [](double x) { return XoQuantity(x, nu::picogram); });
            qty.def("nanograms",  [](double x) { return XoQuantity(x, nu::nanogram); });
            qty.def("micrograms", [](double x) { return XoQuantity(x, nu::microgram); });
            qty.def("milligrams", [](double x) { return XoQuantity(x, nu::milligram); });
            qty.def("grams",      [](double x) { return XoQuantity(x, nu::gram); });
            qty.def("kilograms",  [](double x) { return XoQuantity(x, nu::kilogram); });
            qty.def("tonnes",     [](double x) { return XoQuantity(x, nu::tonne); });
            qty.def("kilotonnes", [](double x) { return XoQuantity(x, nu::kilotonne); });
            qty.def("megaonnes", [](double x) { return XoQuantity(x, nu::megatonne); });

            qty.def("picometers", [](double x) { return XoQuantity(x, nu::picometer); });
            qty.def("nanometers", [](double x) { return XoQuantity(x, nu::nanometer); });
            qty.def("micrometers", [](double x) { return XoQuantity(x, nu::micrometer); });
            qty.def("millimeters", [](double x) { return XoQuantity(x, nu::millimeter); });
            qty.def("meters", [](double x) { return XoQuantity(x, nu::meter); });
            qty.def("kilometers", [](double x) { return XoQuantity(x, nu::kilometer); });
            qty.def("megameters", [](double x) { return XoQuantity(x, nu::megameter); });

            py::class_<XoQuantity>(m, "Quantity")
                .def(py::init<double, const Unit &>(),
                     py::arg("scale"), py::arg("unit"))

                .def("scale", &XoQuantity::scale)
                .def("unit", &XoQuantity::unit)
                .def("is_dimensionless", &XoQuantity::is_dimensionless)
                .def("unit_qty", &XoQuantity::unit_qty)
                .def("zero_qty", &XoQuantity::zero_qty)
                .def("reciprocal", &XoQuantity::reciprocal)
                .def("rescale", &XoQuantity::rescale,
                     py::arg("unit"))

                .def("__add__",
                     [](const XoQuantity & x, const XoQuantity & y)
                         {
                             return x + y;
                         })

                .def("__add__",
                     [](const XoQuantity & x, double y)
                         {
                             return x + y;
                         })

                .def("__radd__",
                     [](double y, const XoQuantity & x)
                         {
                             return x + y;
                         })

                .def("__sub__",
                     [](const XoQuantity & x, const XoQuantity & y)
                         {
                             return x - y;
                         })

                .def("__sub__",
                     [](const XoQuantity & x, double y)
                         {
                             return x - y;
                         })

                .def("__rsub__",
                     [](double y, const XoQuantity & x)
                         {
                             return x - y;
                         })

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

                .def("__eq__",
                     [](const XoQuantity & x, const XoQuantity & y)
                         {
                             return x == y;
                         })

                .def("__eq__",
                     [](const XoQuantity & x, double y)
                         {
                             return x == y;
                         })

                .def("__req__",
                     [](const XoQuantity & y, double x)
                         {
                             return x == y;
                         })

                .def("__repr__",
                     [](const XoQuantity & x)
                         {
                             return tostr(x);
                         })
                .def("abbrev",
                     [](const XoQuantity & x) {
                         return std::string(x.abbrev().c_str());
                     })
                ;
        }
    }
} /*namespace xo*/
