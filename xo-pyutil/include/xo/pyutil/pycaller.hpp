/** @file pycaller.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include <pybind11/pybind11.h>

//#include <cstdint>

namespace xo {
    namespace pyutil {
        struct pycaller_base {
            using void_function_type = void (*)();
            using factory_function_type = pycaller_base*(*)(void_function_type);

            virtual ~pycaller_base() = default;

            /* note: need inherited class pycaller_base revealed to pybind11 too */
            static pybind11::module & declare_once(pybind11::module & m) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pybind11::class_<pycaller_base>(m, "pycaller_base");
                }
                return m;
            }
        };

        /** @class pycaller
         *  @brief Invoke function pointer of type Retval(*)(Args...) from py::object
         *
         *  Arguments converted from py::object, and return type converted back to py::object.
         *
         *  Each distinct combination of {Retval,Args...} needs to be established at compile time
         *  (since we need PyCall<Retval, Args...> to be instantiated for particular types)
         *
         *  Use when we don't know function pointer until *runtime*,
         *  for example getting function pointer from just-compiled code using xo-pyjit
         **/
        template <typename Retval, typename... Args>
        struct pycaller;

        template <typename Retval>
        struct pycaller<Retval> : public pycaller_base {
            using self_type = pycaller<Retval>;
            using function_type = Retval (*)();
            using void_function_type = void (*)();

            pycaller(void_function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pycaller_base * make(void_function_type addr) { return new pycaller(addr); }

            /* note: prototype_str must be [const char *],  pybind11 requirement */
            static pybind11::module & declare_once(pybind11::module & m,
                                                   const char * prototype_str) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, prototype_str)
                        .def("__call__",
                             [](self_type & self)
                                 {
                                     return pybind11::cast((*self.fptr_)());
                                 });
                }
                return m;
            }

            pybind11::object operator()() { return pybind11::cast((*fptr_)()); }

        private:
            function_type fptr_;
        };

        template <typename Retval, typename Arg1>
        struct pycaller<Retval, Arg1> : public pycaller_base {
            using self_type = pycaller<Retval, Arg1>;
            using function_type = Retval (*)(Arg1);
            using void_function_type = void (*)();

            pycaller(void_function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pycaller_base * make(void_function_type addr) { return new pycaller(addr); }

            /* note: prototype_str must be [const char *],  pybind11 requirement */
            static pybind11::module & declare_once(pybind11::module & m,
                                                   const char * prototype_str) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, prototype_str)
                        .def("__call__",
                             [](self_type & self, Arg1 arg1)
                                 {
                                     return pybind11::cast((*self.fptr_)(arg1));
                                 })
                        ;
                }
                return m;
            }

            pybind11::object operator()(pybind11::object arg1) {
                return pybind11::cast((*fptr_)(pybind11::cast<Arg1>(arg1)));
            }

        private:
            function_type fptr_;
        };

        template <typename Retval, typename Arg1, typename Arg2>
        struct pycaller<Retval, Arg1, Arg2> : public pycaller_base {
            using self_type = pycaller<Retval, Arg1, Arg2>;
            using function_type = Retval (*)(Arg1, Arg2);
            using void_function_type = void (*)();

            pycaller(void_function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pycaller_base * make(void_function_type addr) { return new pycaller(addr); }

            /* note: prototype_str must be [const char *],  pybind11 requirement */
            static pybind11::module & declare_once(pybind11::module & m,
                                                   const char * prototype_str) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, prototype_str)
                        .def("__call__",
                             [](self_type & self, Arg1 arg1, Arg2 arg2)
                                 {
                                     return pybind11::cast((*self.fptr_)(arg1, arg2));
                                 })
                        ;
                }
                return m;
            }

            pybind11::object operator()(pybind11::object arg1, pybind11::object arg2) {
                return pybind11::cast((*fptr_)(pybind11::cast<Arg1>(arg1),
                                               pybind11::cast<Arg2>(arg2)));
            }

        private:
            function_type fptr_;
        };
    } /*namespace pyutil*/
}

/** end pycaller.hpp **/
