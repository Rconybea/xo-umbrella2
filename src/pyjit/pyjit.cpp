/* @file pyjit.cpp */

#include "pyjit.hpp"
#include "xo/pyexpression/pyexpression.hpp"
#include "xo/jit/Jit.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/stl.h>

namespace xo {
    struct XferFn : public ref::Refcount {
        using fptr_type = double (*) (double);

        explicit XferFn(fptr_type fptr) : fptr_{fptr} {}

        double operator() (double x) { return (*fptr_)(x); }

        fptr_type fptr_;
    };

    namespace jit {
        using xo::ast::Expression;
        using xo::ref::rp;
        using xo::ref::unowned_ptr;
        namespace py = pybind11;

        PYBIND11_MODULE(XO_PYJIT_MODULE_NAME(), m) {
            // e.g. for xo::ast::Expression
            XO_PYEXPRESSION_IMPORT_MODULE(); // py::module_::import("pyexpression");

            m.doc() = "pybind11 plugin for xo-jit";

            py::class_<Jit, rp<Jit>>(m, "Jit")
                .def_static("make", &Jit::make,
                            py::doc("create Jit instance. Not threadsafe,"
                                    " but does not share resources with any other Jit instance"))

                .def_property_readonly("target_triple", &Jit::target_triple,
                                       py::doc("string describing target host for code generation"))
                .def("get_function_name_v", &Jit::get_function_name_v,
                     py::doc("get vector of function names defined in jit module"))
                .def("dump_execution_session", &Jit::dump_execution_session,
                     py::doc("write to console with state of all jit-owned dynamic libraries"))
                .def("codegen",
                     [](Jit & jit, const rp<Expression> & expr) {
                         return jit.codegen(expr.borrow());
                     },
                     py::arg("x"),
                     py::doc("generate llvm (IR) code for Expression x"),
                     /* we're assuming llvm-generated code lives for as long as the Jit
                      * instance that created it.
                      *
                      * RC 14jun2024 - I think this is true, modulo use of llvm resource trackers.
                      */
                     py::return_value_policy::reference_internal)
                .def("lookup_dbl_dbl_fn",
                     [](Jit & jit, const std::string & symbol) {
                         auto llvm_addr = jit.lookup_symbol(symbol);

                         auto fn_addr = llvm_addr.toPtr<double (*) (double)>();

                         return new XferFn(fn_addr);
                     })
                ;

            py::class_<XferFn, rp<XferFn>>(m, "XferFn")
                .def("__call__",
                     [](XferFn & self, double x) { return self(x); }
                    )
                ;

            py::class_<llvm::Value,
                       unowned_ptr<llvm::Value>>(m, "llvm_Value")
                .def("print",
                     [](llvm::Value & x) {
                         std::string buf;
                         llvm::raw_string_ostream ss(buf);
                         x.print(ss);
                         return buf;
                     })
//                .def("__repr__",
//                     &Jit::display_string)
                ;

        }


    } /*namespace jit*/
} /*namespace xo*/


/* end pyjit.cpp */
