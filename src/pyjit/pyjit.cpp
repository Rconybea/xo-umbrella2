/* @file pyjit.cpp */

#include "pyjit.hpp"
#include "xo/pyexpression/pyexpression.hpp"
#include "xo/jit/MachPipeline.hpp"
#include "xo/pyutil/pycaller.hpp"
#include "xo/pyutil/pyutil.hpp"
#include <llvm/Config/llvm-config.h>
#include <pybind11/stl.h>

namespace xo {
    namespace jit {
        using xo::ast::Expression;
        using xo::pyutil::pycaller_base;
        using xo::pyutil::pycaller;
        using xo::ref::rp;
        //using xo::ref::Refcount;
        using xo::ref::unowned_ptr;
        namespace py = pybind11;

        /** storage for pycaller glue functions for different function signatures.
         *  each pycaller instance embodies captures a canonical (architecture-dependent)
         *  calling sequence for a C/C++ function with that signature.
         **/
        struct pycaller_store {
        public:
            /** singleton instance **/
            static pycaller_store * instance() { return &s_instance; }

            /** establish caller for signature @p prototype_str.
             *  This needs to be called at most once for each distinct signature.
             *
             *  Although it takes module as argument,  the module being used
             *  doesn't (shoudn't ??) matter
             **/
            template <typename Retval, typename... Args>
            pycaller_base::factory_function_type
            require_prototype(py::module & m,
                              const std::string & prototype_str)
                {
                    using caller_type = pycaller<Retval, Args...>;

                    caller_type::declare_once(m);

                    /* factory function takes function pointer of type
                     *   Retval(*)(Args...)
                     * and returns new instance of caller_type for that function
                     */

                    auto ix = pycaller_map_.find(prototype_str);

                    auto retval = &caller_type::make;

                    if(ix == pycaller_map_.end())
                        pycaller_map_[prototype_str] = retval;

                    return retval;
                }

            /** lookup caller for signature @p prototype_str **/
            pycaller_base::factory_function_type
            lookup_prototype(const std::string & prototype_str) const
                {
                    auto ix = pycaller_map_.find(prototype_str);

                    if (ix == pycaller_map_.end())
                        return nullptr;
                    else
                        return ix->second;
                }

        private:
            static pycaller_store s_instance;

            /** map prototype string to pycaller factory for that prototype.
             *  For example
             *    "double(double)" -> pycaller<double,double>()
             **/
            std::unordered_map<std::string,
                               pycaller_base::factory_function_type> pycaller_map_;

        }; /*pycaller_store*/

        pycaller_store
        pycaller_store::s_instance;

        PYBIND11_MODULE(XO_PYJIT_MODULE_NAME(), m) {
            // e.g. for xo::ast::Expression
            XO_PYEXPRESSION_IMPORT_MODULE(); // py::module_::import("pyexpression");

            m.doc() = "pybind11 plugin for xo-jit";

            pycaller_store::instance()
                ->require_prototype<double, double>(m, "double (*)(double)");
            pycaller_store::instance()
                ->require_prototype<double, double, double>(m, "double (*)(double,double)");

            //pycaller<double, double>::declare_once(m);
            //pycaller<double, double, double>::declare_once(m);

            m.def("llvm_version", []() { return LLVM_VERSION_STRING; },
                  py::doc("llvm_version() reports compile-time llvm version string (via [llvm-config.h])"));

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
                .def("dump_current_module", &MachPipeline::dump_current_module,
                     py::doc("Dump contents of current module to console"))


                .def("lookup_fn",
                     [](MachPipeline & jit, const std::string & prototype, const std::string & symbol) -> pycaller_base* {
                         auto llvm_addr = jit.lookup_symbol(symbol);

                         /* llvm doesn't know the actual function signature,
                          * so any function type will appear to succeed here.
                          * We cast to particular function type within the  pycaller<..> template
                          */
                         if (llvm_addr) {
                             auto fn_addr = llvm_addr.get().toPtr<void(*)()>();

                             /* note: llvm_addr.toPtr<..> always succeeds,
                              *       event if pointer refers to an object of incompatible type
                              *
                              * note: return value policy is for python to own the wrapper
                              *
                              * note: pycaller signatures need to have been introduced in advance
                              *       (in practice determined at compile time,
                              *       since they encode a function-signature-specific calling sequence)
                              *       by calling pycaller_store::instance()->require_prototype<Retval, Args...>(prototype);
                              */

                             auto factory = pycaller_store::instance()->lookup_prototype(prototype);

                             if (!factory) {
                                 throw std::runtime_error(tostr("MachPipeline.lookup_fn: unknown function prototype",
                                                                xtag("p", prototype)));
                             }

                             return (*factory)(fn_addr);
                         } else {
                             throw std::runtime_error(tostr("MachPipeline.lookup_fn: lookup on symbol S failed",
                                                            xtag("S", symbol)));
                         }
                     })
                ;


#ifdef OBSOLETE
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
#endif

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
