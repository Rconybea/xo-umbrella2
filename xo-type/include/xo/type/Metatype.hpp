/** @file Metatype.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <iostream>

namespace xo {
    namespace scm {
        class Metatype {
        public:
            enum class code {
                /* void */
                t_unit,

                t_bool,
                t_i64,
                t_f64,
                t_str,

                /* discriminated union */
                t_sum,
                /* list<T> */
                t_list,
                /* array<T> */
                t_array,
                /* function<T(U,V,...)> */
                t_function,
                /* struct<a:T,b:U,..> */
                t_struct,

                /* any type at all */
                t_any,
            };

        public:
            explicit Metatype(code x) : code_{x} {}

            static Metatype unit() { return Metatype(code::t_unit); }

            /** description string for this type category **/
            const char * _descr() const noexcept;

            code code() const noexcept { return code_; }

        private:
            enum code code_;
        };

        inline std::ostream &
        operator<<(std::ostream & os, Metatype x) {
            os << x._descr();
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end Metatype.hpp */
