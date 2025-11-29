/** @file VsmStackFrame.cpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#include "VsmStackFrame.hpp"
#include "xo/reflect/Reflect.hpp"
#include "xo/reflect/StructReflector.hpp"

namespace xo {
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TypeDescrW;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescrExtra;
    using xo::reflect::EstablishTypeDescr;
    using xo::reflect::StlVectorTdx;

    namespace scm {
        namespace {
            // TOOD: move into CVector

            std::size_t
            slot_array_size(std::size_t n) {
                return n * sizeof(gp<Object>);
            }
        }

        VsmStackFrame::VsmStackFrame(gc::IAlloc * mm,
                                     gp<VsmStackFrame> p,
                                     std::size_t n,
                                     const VsmInstr * cont) : parent_{p},
                                                              slot_v_{mm, n},
                                                              cont_{cont}
        {}

        gp<VsmStackFrame>
        VsmStackFrame::make(gc::IAlloc * mm,
                            gp<VsmStackFrame> p,
                            std::size_t n,
                            const VsmInstr * cont)
        {
            gp<VsmStackFrame> retval = new (MMPtr(mm)) VsmStackFrame(mm, p, n, cont);

            for (std::size_t i = 0; i < n; ++i)
                (*retval)[i] = nullptr;

            return retval;
        }

        gp<VsmStackFrame>
        VsmStackFrame::push1(gc::IAlloc * mm,
                             gp<VsmStackFrame> p,
                             gp<Object> s0,
                             const VsmInstr * cont)
        {
            gp<VsmStackFrame> retval = new (MMPtr(mm)) VsmStackFrame(mm, p, 1, cont);

            (*retval)[0] = s0;

            return retval;
        }

        gp<VsmStackFrame>
        VsmStackFrame::push2(gc::IAlloc * mm,
                             gp<VsmStackFrame> p,
                             gp<Object> s0,
                             gp<Object> s1,
                             const VsmInstr * cont)
        {
            gp<VsmStackFrame> retval = new (MMPtr(mm)) VsmStackFrame(mm, p, 2, cont);

            (*retval)[0] = s0;
            (*retval)[1] = s1;

            return retval;
        }

        TaggedPtr
        VsmStackFrame::self_tp() const
        {
            return Reflect::make_tp(const_cast<VsmStackFrame *>(this));
        }

        void
        VsmStackFrame::display(std::ostream & os) const
        {
            os << "<vsm-stack-frame"
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
        VsmStackFrame::_shallow_size() const
        {
            std::size_t retval = sizeof(VsmStackFrame);

            retval += gc::IAlloc::with_padding(slot_array_size(slot_v_.size()));

            return retval;
        }

        Object *
        VsmStackFrame::_shallow_copy(gc::IAlloc * mm) const
        {
            Cpof cpof(mm, this);

            size_t n = this->size();

            VsmStackFrame * copy = new (cpof) VsmStackFrame(cpof.mm_, parent_, n, cont_);

            void * v_dest = copy->slot_v_.v_;

            if (slot_v_.v_) {
                ::memcpy(v_dest, slot_v_.v_, slot_array_size(n));
            }

#ifdef OBSOLETE
            for (size_t i = 0, n = n_slot_; i < n; ++i) {
                copy->v_[i] = v_[i];
            }
#endif
            return copy;
        }

        std::size_t
        VsmStackFrame::_forward_children(gc::IAlloc * gc)
        {
            Object::_forward_inplace(parent_, gc);

            for (std::size_t i = 0, n = slot_v_.size(); i < n; ++i) {
                Object::_forward_inplace((*this)[i], gc);
            }

            return _shallow_size();
        }

        void
        VsmStackFrame::reflect_self()
        {
            StructReflector<VsmStackFrame> sr;

            if (sr.is_incomplete() ) {
                /* reflect CVector<gp<Object>>.
                 * duplicates similar code in LocalEnv::reflect_self()
                 */
                using VectorType = obj::CVector<gp<Object>>;

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

                REFLECT_MEMBER(sr, parent);
                REFLECT_MEMBER(sr, slot_v);
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end VsmStackFrame.cpp */
