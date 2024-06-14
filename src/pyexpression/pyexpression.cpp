/* @file pyexpression.cpp */

#include "pyexpression.hpp"
#include "xo/pyreflect/pyreflect.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/expression/ConstantInterface.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/pyutil/pyutil.hpp"

namespace xo {
    namespace ast {
        using xo::ast::exprtype;
        using xo::ast::Expression;
        using xo::ast::ConstantInterface;
        using xo::ast::Constant;
        using xo::reflect::TaggedPtr;
        using xo::ref::rp;
        namespace py = pybind11;

        PYBIND11_MODULE(XO_PYEXPRESSION_MODULE_NAME(), m) {
            // e.g. for xo::reflect::TypeDescr
            PYREFLECT_IMPORT_MODULE(); // py::module_::import("pyreflect");

            m.doc() = "pybind11 plugin for xo-expression";

            py::enum_<exprtype>(m, "exprtype")
                .value("invalid", exprtype::invalid)
                .value("constant", exprtype::constant)
                .value("primitive", exprtype::primitive)
                .value("apply", exprtype::apply)
                .value("lambda", exprtype::lambda)
                .value("variable", exprtype::variable)
                ;

            py::class_<Expression,
                       rp<Expression>>(m, "Expression")
                .def("extype", &Expression::extype)
                .def("__repr__", &Expression::display_string);
            ;

            py::class_<ConstantInterface,
                       Expression,
                       rp<ConstantInterface>>(m, "ConstantInterface")
                .def("value_td", &ConstantInterface::value_td,
                     py::doc("type description for literal value represented by this Constant"))
                .def("value",
                     [](const ConstantInterface & expr) {
                         TaggedPtr tp = expr.value_tp();

                         auto * p = tp.recover_native<double>();

                         /* TODO: promote to pyobject,  so we can do polymorphism */
                         if (p)
                             return *p;
                         else
                             return std::numeric_limits<double>::quiet_NaN();
                     },
                     py::doc("recover constant expression's wrapped value [wip - only works for double]"))
                ;

            py::class_<Constant<double>, ConstantInterface, rp<Constant<double>>>(m, "Constant_double")
                ;

            m.def("make_constant",
                  [](double x) {
                      return make_constant(x);
                  },
                  py::arg("x"),
                  py::doc("make_constant(x) creates constant expression holding x [wip - only works for double"))
                ;

        } /*pyexpresion*/
    } /*namespace ast*/
} /*namespace xo*/

/* end pyexpression.cpp */
