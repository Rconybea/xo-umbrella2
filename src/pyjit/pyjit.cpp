/* @file pyjit.cpp */

#include "pyjit.hpp"
#include "xo/pyexpression/pyexpression.hpp"
#include "xo/jit/MachPipeline.hpp"
#include "xo/pyutil/pycaller.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <pybind11/stl.h>

namespace xo {
    struct XferDbl2DblFn : public ref::Refcount {
        using fptr_type = double (*) (double);

        explicit XferDbl2DblFn(fptr_type fptr) : fptr_{fptr} {}

        double operator() (double x) { return (*fptr_)(x); }

        fptr_type fptr_;
    }; /*XferDbl2DblFn*/

    struct XferDblDbl2DblFn : public ref::Refcount {
        using fptr_type = double (*) (double, double);

        explicit XferDblDbl2DblFn(fptr_type fptr) : fptr_{fptr} {}

        double operator() (double x, double y) { return (*fptr_)(x, y); }

        fptr_type fptr_;
    }; /*XferDblDbl2DblFn*/

    namespace jit {
        using xo::ast::Expression;
        using xo::pyutil::pycaller_base;
        using xo::pyutil::pycaller;
        using xo::ref::rp;
        //using xo::ref::Refcount;
        using xo::ref::unowned_ptr;
        namespace py = pybind11;

        PYBIND11_MODULE(XO_PYJIT_MODULE_NAME(), m) {
            // e.g. for xo::ast::Expression
            XO_PYEXPRESSION_IMPORT_MODULE(); // py::module_::import("pyexpression");

            m.doc() = "pybind11 plugin for xo-jit";

            pycaller<double, double>::declare_once(m);
            pycaller<double, double, double>::declare_once(m);
            py::class_<MachPipeline, rp<MachPipeline>>(m, "MachPipeline")
                .def_static("make", &MachPipeline::make,
                            py::doc("Create machine pipeline for in-process code generation"
                                    " and execution. Not threadsafe.\n"
                                    "Does not share resources with any other instance"))

                .def_property_readonly("target_triple", &MachPipeline::target_triple,
                                       py::doc("string describing target host for code generation"))
                .def("get_function_name_v", &MachPipeline::get_function_name_v,
                     py::doc("get vector of function names defined in jit module"))
                .def("dump_execution_session", &MachPipeline::dump_execution_session,
                     py::doc("write to console with state of all jit-owned dynamic libraries"))
                .def("codegen",
                     [](MachPipeline & jit, const rp<Expression> & expr) {
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
                .def("machgen_current_module", &MachPipeline::machgen_current_module,
                     py::doc("Make current module available for execution via the jit.\n"
                             "Adds all functions generated since last call to this method."))
                /* double -> double */
                .def("lookup_dbl2dbl_fn",
                     [](MachPipeline & jit, const std::string & symbol) {
                         auto llvm_addr = jit.lookup_symbol(symbol);

                         auto fn_addr = llvm_addr.toPtr<double (*) (double)>();

                         return new XferDbl2DblFn(fn_addr);
                     })

                /* (double x double) -> double */
                .def("lookup_dbldbl2dbl_fn",
                     [](MachPipeline & jit, const std::string & symbol) {
                         auto llvm_addr = jit.lookup_symbol(symbol);

                         auto fn_addr = llvm_addr.toPtr<double (*) (double, double)>();

                         return new XferDblDbl2DblFn(fn_addr);
                     })

                .def("lookup_fn",
                     [](MachPipeline & jit, const std::string & prototype, const std::string & symbol) -> pycaller_base* {
                         auto llvm_addr = jit.lookup_symbol(symbol);

                         /* note: llvm_addr.toPtr<..> always succeeds,
                          *       event if pointer refers to an object of incompatible type
                          *
                          * note: return value policy is for python to own the wrapper
                          */

                         if((prototype == "double(double,double)") || (prototype == "double(*)(double,double)")) {
                             auto fn_addr = llvm_addr.toPtr<double(*)(double,double)>();

                             return new pycaller<double, double, double>(fn_addr);
                             //return new XferDblDbl2DblFn(fn_addr);
                         } else if ((prototype == "double(double)") || (prototype == "double(*)(double)")) {
                             auto fn_addr = llvm_addr.toPtr<double(*)(double)>();

                             return new pycaller<double, double>(fn_addr);
                         } else {
                             throw std::runtime_error(tostr("MachPipeline.lookup_fn: unknown function prototype",
                                                            xtag("p", prototype)));
                         }})
                ;


            py::class_<XferDbl2DblFn, rp<XferDbl2DblFn>>(m, "XferDbl2DblFn")
                .def("__call__",
                     [](XferDbl2DblFn & self, double x) { return self(x); }
                    )
                ;
            py::class_<XferDblDbl2DblFn, rp<XferDblDbl2DblFn>>(m, "XferDblDbl2DblFn")
                .def("__call__",
                     [](XferDblDbl2DblFn & self, double x, double y) { return self(x, y); }
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
