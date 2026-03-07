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
            static Metatype t_bool() { return Metatype(code::t_bool); }
            static Metatype i64() { return Metatype(code::t_i64); }
            static Metatype f64() { return Metatype(code::t_f64); }
            static Metatype str() { return Metatype(code::t_str); }
            static Metatype any() { return Metatype(code::t_any); }

            static Metatype list() { return Metatype(code::t_list); }
            static Metatype array() { return Metatype(code::t_array); }
            static Metatype function() { return Metatype(code::t_function); }

            /** description string for this type category **/
            const char * _descr() const noexcept;

            code code() const noexcept { return code_; }

            /** true iff this metatype is non-parametric:
             *  i.e. stands for a single type
             **/
            bool is_atomic() const noexcept;

        private:
            enum code code_;
        };

        inline bool
        operator==(Metatype x, Metatype y) {
            return (x.code() == y.code());
        }

        inline bool
        operator!=(Metatype x, Metatype y) {
            return (x.code() != y.code());
        }

        inline std::ostream &
        operator<<(std::ostream & os, Metatype x) {
            os << x._descr();
            return os;
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end Metatype.hpp */
