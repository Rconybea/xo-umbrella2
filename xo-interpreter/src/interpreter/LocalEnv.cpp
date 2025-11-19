/** @file LocalEnv.cpp **/

#include "LocalEnv.hpp"
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

        gp<LocalEnv>
        LocalEnv::make(gc::IAlloc * mm,
                       gp<LocalEnv> p,
                       const rp<LocalSymtab> & s,
                       std::size_t n)
        {
            if (s) {
                assert(static_cast<int>(n) == s->n_arg());
            }

            return new (MMPtr(mm)) LocalEnv(mm, p, s, n);
        }

        LocalEnv::LocalEnv(gc::IAlloc * mm,
                           gp<LocalEnv> p,
                           const rp<LocalSymtab> & s,
                           std::size_t n) : parent_{p},
                                            symtab_{s},
                                            slot_v_{mm, n}
        {}

        TaggedPtr
        LocalEnv::self_tp() const
        {
            return Reflect::make_tp(const_cast<LocalEnv *>(this));
        }

        void
        LocalEnv::display(std::ostream & os) const
        {
            os << "<local-env"
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
        LocalEnv::_shallow_size() const
        {
            std::size_t retval = sizeof(LocalEnv);

            retval += gc::IAlloc::with_padding(slot_array_size(slot_v_.size()));

            return retval;
        }

        Object *
        LocalEnv::_shallow_copy() const
        {
            Cpof cpof(Object::mm, this);

            size_t z = size();

            LocalEnv * copy = new (cpof) LocalEnv(cpof.mm_, parent_, symtab_, z);

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
        LocalEnv::_forward_children()
        {
            static_assert(decltype(symtab_)::is_gc_ptr == false);

            Object::_forward_inplace(parent_);
            // Object::_forward_inplace(symtab_);  // not a gp yet
            for (std::size_t i = 0, n = slot_v_.size(); i < n; ++i) {
                Object::_forward_inplace((*this)[i]);
            }

            return _shallow_size();
        }

        void
        LocalEnv::reflect_self()
        {
            StructReflector<LocalEnv> sr;

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

                REFLECT_MEMBER(sr, parent);
                REFLECT_MEMBER(sr, slot_v);
            }
        }
    } /*namespace scm*/
} /*namespace xo*/

/* end LocalEnv.cpp */
