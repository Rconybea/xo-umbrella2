/** @file Metatype.cpp
 *
 *  @author Roland Conybeare, Nar 2026
 **/

#include "Metatype.hpp"

namespace xo {
    namespace scm {

        const char *
        Metatype::_descr() const noexcept
        {
            switch (code_) {
            case code::t_any:      return "any";
            case code::t_bool:     return "bool";
            case code::t_i64:      return "i64";
            case code::t_f64:      return "f64";
            case code::t_str:      return "str";
            case code::t_sum:      return "sum";
            case code::t_list:     return "list";
            case code::t_array:    return "array";
            case code::t_function: return "function";
            case code::t_struct:   return "struct";
            case code::t_unit:     return "unit";
            }
        }

        bool
        Metatype::is_atomic() const noexcept
        {
            switch (code_) {
            case code::t_any:
                return true;
            case code::t_bool:
                return true;
            case code::t_i64:
                return true;
            case code::t_f64:
                return true;
            case code::t_str:
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
            case code::t_unit:
                return false;
            }
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end Metatype.cpp */
