/** @file Metatype.cpp
 *
 *  @author Roland Conybeare, Nar 2026
 **/

#include "Metatype.hpp"
#include <cassert>

namespace xo {
    namespace scm {

        const char *
        Metatype::_descr() const noexcept
        {
            switch (code_) {
            case code::t_unit:     return "unit";
            case code::t_bool:     return "bool";
            case code::t_str:      return "str";

            case code::t_i16:      return "i16";
            case code::t_i32:      return "i32";
            case code::t_i64:      return "i64";

            case code::t_f32:      return "f32";
            case code::t_f64:      return "f64";

            case code::t_sum:      return "sum";
            case code::t_list:     return "list";
            case code::t_array:    return "array";
            case code::t_function: return "function";
            case code::t_struct:   return "struct";
            case code::t_dict:     return "dict";

            case code::t_integer:  return "integer";
            case code::t_booleic:  return "booleic";
            case code::t_numeric:  return "numeric";
            case code::t_callable: return "callable";
            case code::t_any:      return "any";
            }

            assert(false);
            return "?metatype";
        }

        bool
        Metatype::is_atomic() const noexcept
        {
            switch (code_) {
            case code::t_unit:
                return false;

            case code::t_bool:
                return true;
            case code::t_str:
                return true;

            case code::t_i16:
                return true;
            case code::t_i32:
                return true;
            case code::t_i64:
                return true;

            case code::t_f32:
                return true;
            case code::t_f64:
                return true;

            case code::t_sum:
                return false;
            case code::t_list:
                return false;
            case code::t_array:
                return false;
            case code::t_function:
                return false;
            case code::t_struct:
                return false;
            case code::t_dict:
                return true;

            case code::t_integer:
                return true;
            case code::t_booleic:
                return true;
            case code::t_numeric:
                return true;
            case code::t_callable:
                return true;
            case code::t_any:
                return true;
            }

            assert(false);
            return false;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end Metatype.cpp */
