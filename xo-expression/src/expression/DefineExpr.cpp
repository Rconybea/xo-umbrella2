/* file DefineExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "DefineExpr.hpp"
#include "Variable.hpp"

namespace xo {
    namespace ast {
        rp<DefineExpr>
        DefineExpr::make(std::string lhs_name,
                         rp<Expression> rhs)
        {
            TypeDescr rhs_valuetype = nullptr;

            if (rhs)
                rhs_valuetype = rhs->valuetype();

            return new DefineExpr(rhs_valuetype,
                                  std::move(lhs_name),
                                  std::move(rhs));
        } /*make*/

        DefineExpr::DefineExpr(TypeDescr rhs_valuetype,
                               std::string lhs_name,
                               rp<Expression> rhs)
            : Expression(exprtype::define, rhs_valuetype),
              lhs_name_{std::move(lhs_name)},
              rhs_{std::move(rhs)}
        {
            this->free_var_set_ = this->calc_free_variables();
        }

        rp<Variable>
        DefineExpr::lhs_variable() const
        {
            return Variable::make(lhs_name(), valuetype());
        }

        std::set<std::string>
        DefineExpr::calc_free_variables() const
        {
            std::set<std::string> retval;

            if (rhs_)
                retval = rhs_->get_free_variables();

            /* but remove this variable */
            if (!this->lhs_name().empty())
                retval.erase(this->lhs_name());

            return retval;
        } /*calc_free_variables*/

        void
        DefineExpr::display(std::ostream & os) const {
            os << "<Define"
               << xtag("name", lhs_name_)
               << xtag("rhs", rhs_)
               << ">";
        } /*display*/

        // ----- DefineExprAccess -----

        rp<DefineExprAccess>
        DefineExprAccess::make(std::string lhs_name,
                               rp<Expression> rhs)
        {
            TypeDescr rhs_valuetype = nullptr;

            if (rhs)
                rhs_valuetype = rhs->valuetype();

            return new DefineExprAccess(rhs_valuetype,
                                        std::move(lhs_name),
                                        std::move(rhs));
        }

        rp<DefineExprAccess>
        DefineExprAccess::make_empty()
        {
            return new DefineExprAccess(nullptr /*rhs_valuetype*/,
                                        "" /*lhs_name*/,
                                        nullptr /*rhs*/);
        }

        void
        DefineExprAccess::assign_rhs(const rp<Expression> & x)
        {
            assert(x);

            this->rhs_ = x;

            if (x) {
                this->assign_valuetype(x->valuetype());
            }

            this->free_var_set_ = this->calc_free_variables();
        }

    } /*namespace ast*/
} /*namespace xo*/


/* end DefineExpr.cpp */
