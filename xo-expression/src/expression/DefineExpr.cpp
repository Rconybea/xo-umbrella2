/* file DefineExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "DefineExpr.hpp"
#include "Variable.hpp"
#include <cstdint>
#include <xo/ppsink/tag_ostream.hpp>
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::field;
    using xo::pp::xtag;
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
        DefineExpr::pretty(xo::pp::PpSink & sink) const
        {
            sink.pretty_struct("Define",
                               //field("type", this->valuetype()), // need pretty
                               field("name", lhs_var_->name()),
                               field("rhs", rhs_));
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
