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
            virtual ~pycaller_base() = default;
g
            static pybind11::module & declare_once(pybind11::module & m) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pybind11::class_<pycaller_base>(m, "pycaller_base");
                }
                return m;
            }
        };

        /** Invoke function pointer of type Retval(*)(Args...),
         *  with arguments converted from py::object, and return type converted back to py::object.
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

            pycaller(function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pybind11::module & declare_once(pybind11::module & m) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, "pycaller0")
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

            pycaller(function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pybind11::module & declare_once(pybind11::module & m) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, "pycaller1")
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

            pycaller(function_type addr) : fptr_{reinterpret_cast<function_type>(addr)} {}

            static pybind11::module & declare_once(pybind11::module & m) {
                static bool s_once = false;
                if (!s_once) {
                    s_once = true;
                    pycaller_base::declare_once(m);
                    pybind11::class_<self_type, pycaller_base>(m, "pycaller2")
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
