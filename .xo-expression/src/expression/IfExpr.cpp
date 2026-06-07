/* @file IfExpr.cpp */

#include "IfExpr.hpp"
#include "pretty_expression.hpp"
#include "pretty_variable.hpp"
//#include "xo/indentlog/print/vector.hpp"

namespace xo {
    namespace scm {
        auto IfExpr::check_consistent_valuetype(const rp<Expression> & when_true,
                                                const rp<Expression> & when_false) -> TypeDescr
        {
            if (when_true->valuetype() != when_false->valuetype())
                return nullptr;

            return when_true->valuetype();
        }

        void IfExpr::establish_valuetype()
        {
            if (this->when_true_.get() && this->when_false_.get())
                this->assign_valuetype(check_consistent_valuetype(this->when_true_, this->when_false_));
        }

        rp<IfExpr>
        IfExpr::make(const rp<Expression> & test,
                     const rp<Expression> & when_true,
                     const rp<Expression> & when_false)
        {
            /** TODO: verify test returns _boolean_ type **/

            if (when_true->valuetype() != when_false->valuetype()) {
                throw std::runtime_error
                    (tostr("IfExpr::make:"
                           " types {T1,T2} found for branches of if-expr"
                           " where equal types expected",
                           xtag("T1", when_true->valuetype()->canonical_name()),
                           xtag("T2", when_false->valuetype()->canonical_name())));
            }

            /* arbitrary choice here */
            auto ifexpr_type = when_true->valuetype();

            return new IfExpr(ifexpr_type,
                              test,
                              when_true,
                              when_false);
        } /*make*/

        void
        IfExpr::display(std::ostream & os) const {
            os << "<IfExpr"
               << xtag("test", test_)
               << xtag("when_true", when_true_);
            if (when_false_)
                os << xtag("when_false", when_false_);
            os << ">";
        } /*display*/

        std::uint32_t
        IfExpr::pretty_print(const ppindentinfo & ppii) const {
            return ppii.pps()->pretty_struct(ppii, "IfExpr",
                                             refrtag("test", test_),
                                             refrtag("when_true", when_true_),
                                             refrtag("when_false", when_false_));
        }

        rp<IfExprAccess>
        IfExprAccess::make(rp<Expression> test,
                           rp<Expression> when_true,
                           rp<Expression> when_false)
        {
            auto ifexpr_type = check_consistent_valuetype(when_true, when_false);

            return new IfExprAccess(ifexpr_type, std::move(test), std::move(when_true), std::move(when_false));
        }

        rp<IfExprAccess>
        IfExprAccess::make_empty()
        {
            return new IfExprAccess(nullptr /*ifexpr_valuetype*/,
                                    nullptr /*test*/,
                                    nullptr /*when_true*/,
                                    nullptr /*when_false*/);
        }

        void
        IfExprAccess::assign_when_true(rp<Expression> x)
        {
            this->when_true_ = std::move(x);
        }

        void
        IfExprAccess::assign_when_false(rp<Expression> x)
        {
            this->when_false_ = std::move(x);
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end IfExpr.cpp */
