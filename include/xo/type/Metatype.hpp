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
                t_str,

                /* int16_t */
                t_i16,
                /* int32_t */
                t_i32,
                /* int64_t */
                t_i64,

                /* float */
                t_f32,
                /* double */
                t_f64,

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
                /* dicttionary: like struct, but w/ dynamic key/value pairs  */
                t_dict,

                /** any integer type: i16|i32|i64 **/
                t_integer,

                /** any numeric type: i16|i32|i64|f32|f64 **/
                t_numeric,

                /** generalized boolean type: bool, function with bool codomain **/
                t_booleic,

                /** any callable type (e.g. all function types) **/
                t_callable,

                /* any type at all */
                t_any,
            };

        public:
            explicit Metatype(code x) : code_{x} {}

            static Metatype t_unit()     { return Metatype(code::t_unit);     }
            static Metatype t_bool()     { return Metatype(code::t_bool);     }
            static Metatype t_str()      { return Metatype(code::t_str);      }

            static Metatype t_i16()      { return Metatype(code::t_i16);      }
            static Metatype t_i32()      { return Metatype(code::t_i32);      }
            static Metatype t_i64()      { return Metatype(code::t_i64);      }

            static Metatype t_f32()      { return Metatype(code::t_f32);      }
            static Metatype t_f64()      { return Metatype(code::t_f64);      }

            static Metatype t_sum()      { return Metatype(code::t_sum);      }
            static Metatype t_list()     { return Metatype(code::t_list);     }
            static Metatype t_array()    { return Metatype(code::t_array);    }
            static Metatype t_function() { return Metatype(code::t_function); }
            static Metatype t_struct()   { return Metatype(code::t_struct);   }

            static Metatype t_dict()     { return Metatype(code::t_dict);     }
            static Metatype t_integer()  { return Metatype(code::t_integer);  }
            static Metatype t_numeric()  { return Metatype(code::t_numeric);  }
            static Metatype t_booleic()  { return Metatype(code::t_booleic);  }
            static Metatype t_callable() { return Metatype(code::t_callable); }
            static Metatype t_any()      { return Metatype(code::t_any);      }

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
