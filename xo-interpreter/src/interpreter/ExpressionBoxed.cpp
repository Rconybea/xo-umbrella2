/** @file ExpressionBoxed.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "ExpressionBoxed.hpp"
#include <xo/reflect/Reflect.hpp>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;

    namespace scm {
        ExpressionBoxed::ExpressionBoxed(bp<Expression> c) : contents_{c.promote()}
        {}

        gp<ExpressionBoxed>
        ExpressionBoxed::make(gc::IAlloc * mm,
                              bp<Expression> c)
        {
            return new (MMPtr(mm)) ExpressionBoxed(c);
        }


        TaggedPtr
        ExpressionBoxed::self_tp() const
        {
            return Reflect::make_tp(const_cast<ExpressionBoxed *>(this));
        }

        void
        ExpressionBoxed::pretty(xo::pp::PpSink & sink) const
        {
            /* was: os << contents_.
             *
             * Deliberately still a bare delegation: ExpressionBoxed is a box,
             * and its rendering is whatever it wraps.  Do not add framing of
             * its own here.  Going through the sink rather than an ostream
             * means the boxed value now nests instead of flattening.
             */
            sink.pp(contents_);
        }

        std::size_t
        ExpressionBoxed::_shallow_size() const
        {
            return sizeof(ExpressionBoxed);
        }

        Object *
        ExpressionBoxed::_shallow_copy(gc::IAlloc * mm) const
        {
            Cpof cpof(mm, this);

            return new (cpof) ExpressionBoxed(*this);
        }

        std::size_t
        ExpressionBoxed::_forward_children(gc::IAlloc *)
        {
            return _shallow_size();
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end ExpressionBoxed.cpp */
