/* file ConvertExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "ConvertExpr.hpp"

namespace xo {
    namespace ast {
        rp<ConvertExpr>
        ConvertExpr::make(TypeDescr dest_type,
                          rp<Expression> arg)
        {
            return new ConvertExpr(dest_type,
                                   std::move(arg));
        }

        std::set<std::string>
        ConvertExpr::get_free_variables() const {
            if (this->arg_)
                return this->arg_->get_free_variables();
            else
                return std::set<std::string>();
        }

        void
        ConvertExpr::display(std::ostream & os) const {
            os << "<Convert"
               << xtag("dest_type", this->valuetype()->short_name())
               << xtag("arg", arg_)
               << ">";
        }

        // ----- ConvertExprAccess -----

        rp<ConvertExprAccess>
        ConvertExprAccess::make(TypeDescr dest_type,
                                rp<Expression> arg)
        {
            return new ConvertExprAccess(dest_type,
                                         std::move(arg));
        }

        rp<ConvertExprAccess>
        ConvertExprAccess::make_empty() {
            return new ConvertExprAccess(nullptr /*dest_type*/,
                                         nullptr /*arg*/);
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end ConvertExpr.cpp */
