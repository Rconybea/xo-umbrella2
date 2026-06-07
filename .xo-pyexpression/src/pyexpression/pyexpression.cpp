/* @file pyexpression.cpp */

#include "pyexpression.hpp"
#include "xo/pyreflect/pyreflect.hpp"
#include "xo/expression/Expression.hpp"
#include "xo/expression/Apply.hpp"
#include "xo/expression/PrimitiveExprInterface.hpp"
#include "xo/expression/PrimitiveExpr.hpp"
#include "xo/expression/ConstantInterface.hpp"
#include "xo/expression/Constant.hpp"
#include "xo/expression/Variable.hpp"
#include "xo/expression/Lambda.hpp"
#include "xo/expression/IfExpr.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <cmath>

namespace xo {
    namespace scm {
        using xo::scm::exprtype;
        using xo::scm::Expression;
        using xo::scm::make_apply;
        using xo::scm::PrimitiveExprInterface;
        using xo::scm::PrimitiveExpr;
        using xo::scm::make_primitive;
        using xo::scm::ConstantInterface;
        using xo::scm::Constant;
        using xo::scm::Variable;
        using xo::scm::make_var;
        using xo::scm::Lambda;
        using xo::scm::make_lambda;
        using xo::scm::IfExpr;
        using xo::scm::make_ifexpr;
        using xo::reflect::TaggedPtr;
        using xo::rp;
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
                .value("ifexpr", exprtype::ifexpr)
                ;

            py::class_<Expression,
                       rp<Expression>>(m, "Expression")
                .def_property_readonly("extype", &Expression::extype)
                .def_property_readonly("valuetype", &Expression::valuetype)
                .def("get_free_variables", &Expression::get_free_variables)
                .def("__repr__", &Expression::display_string);
            ;

            // ----- Constants -----

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

            // ----- Primitives -----

            py::class_<PrimitiveExprInterface,
                       Expression,
                       rp<PrimitiveExprInterface>>(m, "PrimitiveInterface")
                .def("name", &PrimitiveExprInterface::name,
                     py::doc("name of this primitive function;  use this name to invoke the function"))
                .def("n_arg", &PrimitiveExprInterface::n_arg,
                     py::doc("number of arguments to this function (not counting return value)"))
                ;

            using int32_t = std::int32_t;

            py::class_<PrimitiveExpr<int32_t (*)(int32_t, int32_t)>,
                       PrimitiveExprInterface,
                       rp<PrimitiveExpr<int32_t (*)(int32_t, int32_t)>>>(m, "PrimitiveExpr_i32_i32")
                ;

            py::class_<PrimitiveExpr<double (*)(double)>,
                       PrimitiveExprInterface,
                       rp<PrimitiveExpr<double (*)(double)>>>(m, "PrimitiveExpr_double_double")
                ;
            using Fn_dbl_dbl_type = double (*)(double);

            m.def("make_sqrt_pm", []() { return make_primitive<double (*)(double)>("sqrt",
                                                                                   ::sqrt,
                                                                                   false /*!explicit_symbol_def*/,
                                                                                   llvmintrinsic::fp_sqrt); },
                  py::doc("create primitive representing the ::sqrt() function"));
            m.def("make_sin_pm",
                  []() { return make_primitive<Fn_dbl_dbl_type>("sin",
                                                                ::sin,
                                                                false /*!explicit_symbol_def*/,
                                                                llvmintrinsic::fp_sin); },
                  py::doc("create primitive representing the ::sin() function"));
            m.def("make_cos_pm",
                  []() { return make_primitive<Fn_dbl_dbl_type>("cos",
                                                                ::cos,
                                                                false /*!explicit_symbol_def*/,
                                                                llvmintrinsic::fp_cos); },
                  py::doc("create primitive representing the ::cos() function"));

            py::class_<PrimitiveExpr<double (*)(double, double)>,
                       PrimitiveExprInterface,
                       rp<PrimitiveExpr<double (*)(double, double)>>>(m, "PrimitiveExpr_double_double_double")
                ;

            m.def("make_pow_pm",
                  []() { return make_primitive<double (*)(double, double)>("pow",
                                                                           ::pow,
                                                                           false /*!explicit_symbol_def*/,
                                                                           llvmintrinsic::fp_pow); },
                  py::doc("create primitive representing the ::pow() function"));

            // ----- Apply -----

            py::class_<Apply, Expression, rp<Apply>>(m, "Apply")
                .def_property_readonly("fn", &Apply::fn, py::doc("function to be invoked"))
                .def_property_readonly("argv", &Apply::argv, py::doc("expressions (in position order) for function arguments"))
                ;

            m.def("make_apply", &Apply::make);

            // ----- Variables -----

            py::class_<Variable, Expression, rp<Variable>>(m, "Variable")
                .def_property_readonly("name", &Variable::name, py::doc("variable name"))
                ;

            m.def("make_var", &make_var,
                  py::arg("name"),
                  py::arg("var_type"));

            // ----- Lambdas -----

            py::class_<Lambda, Expression, rp<Lambda>>(m, "Lambda")
                .def_property_readonly("name", &Lambda::name, py::doc("lambda name (maybe automatically generated?)"))
                .def_property_readonly("type_str", &Lambda::type_str, py::doc("string specifying lambda type. e.g. \"double(double,double)\""))
                .def_property_readonly("argv", &Lambda::argv, py::doc("lambda formal parameters"))
                .def_property_readonly("body", &Lambda::body, py::doc("lambda body expression"))
                .def_property_readonly("n_arg", &Lambda::n_arg, py::doc("number of format parameters to this lambda function"))
                ;

            m.def("make_lambda", &make_lambda);

            // ----- IfExpr -----

            py::class_<IfExpr, Expression, rp<IfExpr>>(m, "IfExpr")
                .def_property_readonly("test", &IfExpr::test, py::doc("test expression"))
                .def_property_readonly("when_true", &IfExpr::when_true, py::doc("execute this expression when (and only when) test succeeds"))
                .def_property_readonly("when_false", &IfExpr::when_false, py::doc("execute this expression when (and only when) test fails"))
                ;

            m.def("make_ifexpr", &make_ifexpr);
        } /*pyexpresion*/
    } /*namespace scm*/
} /*namespace xo*/

/* end pyexpression.cpp */
