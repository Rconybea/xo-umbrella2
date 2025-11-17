/** @file StackFrame.cpp **/

#include "StackFrame.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/StructReflector.hpp"
#include <cstring>

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TypeDescrW;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescrExtra;
    using xo::reflect::EstablishTypeDescr;
    using xo::reflect::StlVectorTdx;
    using xo::print::quot;

    namespace scm {
        namespace {
            std::size_t
            slot_array_size(std::size_t n) {
                return n * sizeof(gp<Object>);
            }
        }

        gp<StackFrame>
        StackFrame::make(gc::IAlloc * mm, std::size_t n)
        {
            return new (MMPtr(mm)) StackFrame(mm, n);
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
               << xtag("n", slot_v_.size());

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

            retval += gc::IAlloc::with_padding(slot_array_size(slot_v_.size()));

            return retval;
        }

        Object *
        StackFrame::_shallow_copy() const
        {
            Cpof cpof(Object::mm, this);

            size_t z = size();

            StackFrame * copy = new (cpof) StackFrame(cpof.mm_, z);

            void * v_dest = copy->slot_v_.v_;

            if (slot_v_.v_) {
                ::memcpy(v_dest, slot_v_.v_, slot_array_size(z));
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
            for (std::size_t i = 0, n = slot_v_.size(); i < n; ++i) {
                Object::_forward_inplace((*this)[i]);
            }

            return _shallow_size();
        }

        void
        StackFrame::reflect_self()
        {
            StructReflector<StackFrame> sr;

            if (sr.is_incomplete()) {
                /* reflect CVector<gp<Object>>
                 *
                 * note: placement here works b/c CVector<T> not used anywhere else
                 */
                using VectorType = CVector<gp<Object>>;

                /* custom reflection for array of Object pointers.
                 * Can use StlVectorTdx here, treating CVector<T> as a vector
                 * via .size() and .operator[] members
                 */
                std::unique_ptr<TypeDescrExtra> tdx1
                    = std::make_unique<StlVectorTdx<VectorType>>();
                TypeDescrW td1
                    = EstablishTypeDescr::establish<VectorType>();
                td1->assign_tdextra(Reflect::get_final_invoker<VectorType>(),
                                    std::move(tdx1));

                REFLECT_MEMBER(sr, slot_v);
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end StackFrame.cpp */
