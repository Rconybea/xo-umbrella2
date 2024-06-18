/* @file IfExpr.cpp */

#include "IfExpr.hpp"
#include "xo/indentlog/print/vector.hpp"

namespace xo {
    using xo::ref::rp;

    namespace ast {
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
               << xtag("when_true", when_true_)
               << xtag("when_false", when_false_)
               << ">";
        } /*display*/
    } /*namespace ast*/
} /*namespace xo*/


/* end IfExpr.cpp */
