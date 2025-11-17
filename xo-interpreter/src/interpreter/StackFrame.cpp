/** @file StackFrame.cpp **/

#include "StackFrame.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/StructReflector.hpp"
#include <cstring>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;
    using xo::print::quot;

    namespace scm {
        namespace {
            std::size_t
            slot_array_size(std::size_t n) {
                return n * sizeof(gp<Object>);
            }
        }

        StackFrame::StackFrame(gc::IAlloc * mm, std::size_t n_slot)
            : n_slot_{n_slot}, v_{nullptr}
        {
            if (n_slot > 0) {
                std::byte * mem = mm->alloc(slot_array_size(n_slot));

                this->v_ = new (mem) gp<Object>[n_slot];
            }
        }

        gp<StackFrame>
        StackFrame::make(gc::IAlloc * mm, std::size_t n_slot)
        {
            return new (MMPtr(mm)) StackFrame(mm, n_slot);
        }

        TaggedPtr
        StackFrame::self_tp() const
        {
            return Reflect::make_tp(const_cast<StackFrame *>(this));
        }

        void
        StackFrame::display(std::ostream & os) const
        {
            os << "<stack-frame"
               << xtag("n_slot", n_slot_);

#ifdef NOT_YET
            for (std::size_t i = 0, n = n_slot(); i < n; ++i) {
                char buf[24];
                snprintf(buf, sizeof(buf), "v[%lu]", i);

                os << xtag(buf, lookup(i));
            }
#endif

            os << ">";
        }

        std::size_t
        StackFrame::_shallow_size() const
        {
            std::size_t retval = sizeof(StackFrame);

            retval += gc::IAlloc::with_padding(slot_array_size(n_slot_));

            return retval;
        }

        Object *
        StackFrame::_shallow_copy() const
        {
            Cpof cpof(Object::mm, this);

            StackFrame * copy = new (cpof) StackFrame(cpof.mm_, n_slot_);

            void * v_dest = copy->v_;

            if (v_) {
                ::memcpy(v_dest, v_, slot_array_size(n_slot_));
            }

#ifdef OBSOLETE
            for (size_t i = 0, n = n_slot_; i < n; ++i) {
                copy->v_[i] = v_[i];
            }
#endif

            return copy;
        }

        std::size_t
        StackFrame::_forward_children()
        {
            for (std::size_t i = 0, n = n_slot_; i < n; ++i) {
                Object::_forward_inplace(lookup(i));
            }

            return _shallow_size();
        }

        void
        StackFrame::reflect_self()
        {
            StructReflector<StackFrame> sr;

            if (sr.is_incomplete()) {
                REFLECT_MEMBER(sr, n_slot);

                // non-trivial to reflect frame members,
                // effectively need separate reflection for each cardinality;
                // or: reflect .v_[] as nested element
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end StackFrame.cpp */
