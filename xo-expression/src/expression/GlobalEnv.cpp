/* file GlobalEnv.cpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#include "GlobalEnv.hpp"
#include "Expression.hpp"

namespace xo {
    namespace ast {
        GlobalEnv::GlobalEnv() = default;

        bp<Expression>
        GlobalEnv::require_global(const std::string & vname,
                                  bp<Expression> expr)
        {
            this->global_map_[vname] = expr.get();
            return expr;
        } /*require_global*/

        void
        GlobalEnv::print(std::ostream & os) const {
            os << "<globalenv" << xtag("size", global_map_.size()) << ">";
        }
    } /*namespace ast*/
} /*namespace xo*/
