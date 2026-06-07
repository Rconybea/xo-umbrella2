/* file DefineExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "DefineExpr.hpp"
#include "Variable.hpp"
#include "pretty_expression.hpp"
#include <cstdint>

namespace xo {
    namespace scm {
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
              lhs_var_{Variable::make(lhs_name, rhs_valuetype)},
              rhs_{std::move(rhs)}
        {
            this->free_var_set_ = this->calc_free_variables();
        }

        const std::string &
        DefineExpr::lhs_name() const { return lhs_var_->name(); }

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
               << xtag("name", lhs_var_->name())
               << xtag("rhs", rhs_)
               << ">";
        } /*display*/

        std::uint32_t
        DefineExpr::pretty_print(const ppindentinfo & ppii) const
        {
            return ppii.pps()->pretty_struct(ppii, "Define",
                                             //refrtag("type", this->valuetype()), // need pretty
                                             refrtag("name", lhs_var_->name()),
                                             refrtag("rhs", rhs_));
        }

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
        DefineExprAccess::assign_lhs_name(const std::string & x)
        {
            this->lhs_var_->assign_name(x);
        }

        void
        DefineExprAccess::assign_rhs(const rp<Expression> & x)
        {
            assert(x);

            this->rhs_ = x;

            if (x) {
                if (lhs_var_ && !lhs_var_->valuetype()) {
                    this->lhs_var_->assign_valuetype(x->valuetype());
                }

                this->assign_valuetype(x->valuetype());
            }

            this->free_var_set_ = this->calc_free_variables();
        }

    } /*namespace scm*/
} /*namespace xo*/


/* end DefineExpr.cpp */
