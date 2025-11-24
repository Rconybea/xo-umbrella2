/** @file ExpressionBoxed.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/Object.hpp"
#include "xo/expression/Expression.hpp"

namespace xo {
    namespace scm {
        /** @class ExpressionBoxed
         *  @brief xo::scm::Expression, adapted to xo::Object interface
         **/
        class ExpressionBoxed : public Object {
        public:
            explicit ExpressionBoxed(bp<Expression> c);

            /** create boxed version of @p c, using allocator @p mm **/
            static gp<ExpressionBoxed> make(gc::IAlloc * mm,
                                            bp<Expression> c);

            const rp<Expression> & contents() const { return contents_; }

            // inherited from Object
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * mm) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * /*gc*/) final override;

        private:
            /** reference-counted Expression pointer
             *
             *  NOTE correctness requires finalization support in xo::gc::GC
             **/
            rp<Expression> contents_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end ExpressionBoxed.hpp */
