/* file AssignExpr.cpp
 *
 * author: Roland Conybeare
 */

#include "AssignExpr.hpp"
#include "pretty_expression.hpp"
#include "pretty_variable.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <cstdint>

namespace xo {
    namespace ast {
        rp<AssignExpr>
        AssignExpr::make(const rp<Variable> & lhs,
                         const rp<Expression> & rhs)
        {
            return new AssignExpr(lhs, rhs);
        }

        AssignExpr::AssignExpr(const rp<Variable> & lhs,
                               const rp<Expression> & rhs)
            : Expression(exprtype::assign, rhs->valuetype()),
              lhs_{lhs}, rhs_{rhs}
        {
            this->free_var_set_ = this->calc_free_variables();
        }

        std::set<std::string>
        AssignExpr::calc_free_variables() const
        {
            std::set<std::string> retval = lhs_->get_free_variables();

            std::set<std::string> tmp = rhs_->get_free_variables();

            for (const auto & name : tmp)
                retval.insert(name);

            return retval;
        }

        std::set<std::string>
        AssignExpr::get_free_variables() const {
            return free_var_set_;
        }

        std::size_t
        AssignExpr::visit_preorder(VisitFn visitor_fn) {
            std::size_t n = 1;

            visitor_fn(this);

            n += lhs_->visit_preorder(visitor_fn);
            n += rhs_->visit_preorder(visitor_fn);

            return n;
        }

        std::size_t
        AssignExpr::visit_layer(VisitFn visitor_fn) {
            std::size_t n = 1;

            visitor_fn(this);

            n += lhs_->visit_layer(visitor_fn);
            n += rhs_->visit_layer(visitor_fn);

            return n;
        }

        rp<Expression>
        AssignExpr::xform_layer(TransformFn xform_fn) {
            this->lhs_ = Variable::from(lhs_->xform_layer(xform_fn)).promote();
            this->rhs_ = rhs_->xform_layer(xform_fn);

            return xform_fn(this);
        }

         void
         AssignExpr::attach_envs(bp<Environment> p) {
             lhs_->attach_envs(p);
             rhs_->attach_envs(p);
         }

        void
        AssignExpr::display(std::ostream & os) const {
            os << "<Assign"
               << xtag("lhs", lhs_)
               << xtag("rhs", rhs_)
               << ">";
        }

        std::uint32_t
        AssignExpr::pretty_print(const ppindentinfo & ppii) const {
            return ppii.pps()->pretty_struct(ppii, "AssignExpr",
                                             refrtag("lhs", lhs_),
                                             refrtag("rhs", rhs_));
        }
    } /*namespace ast*/
} /*namespace xo*/


/* end AssignExpr.cpp */
