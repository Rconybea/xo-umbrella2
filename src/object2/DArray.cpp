/** @file DArray.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArray.hpp"
#include <xo/indentlog/print/tostr.hpp>
#include <xo/indentlog/print/tag.hpp>
#include <cstdint>

namespace xo {
    using xo::mm::AGCObject;
    using xo::facet::typeseq;

    namespace scm {
        DArray *
        DArray::empty(obj<AAllocator> mm,
                      size_type cap)
        {
            assert(cap > 0);

            DArray * result = nullptr;

            if (cap > 0) {
                void * mem = mm.alloc(typeseq::id<DArray>(),
                                      sizeof(DArray) + cap * sizeof(obj<AGCObject>));

                result = new (mem) DArray();

                assert(result);

                result->capacity_ = cap;
                result->size_ = 0;
            }

            return result;
        }

        obj<AGCObject>
        DArray::at(size_type ix) const
        {
            if (ix < size_) {
                return elts_[ix];
            } else {
                throw std::runtime_error(tostr("DArray::at: out-of-range index where [0..z) expected",
                                               xtag("index", ix),
                                               xtag("z", this->size())));
                return obj<AGCObject>();
            }
        }

        bool
        DArray::push_back(obj<AGCObject> elt) noexcept {
            if (size_ >= capacity_) {
                return false;
            } else {
                static_assert(!std::is_trivially_constructible_v<obj<AGCObject>>);

                void * mem = &(elts_[size_]);

                new (mem) obj<AGCObject>(elt);

                ++(this->size_);

                return true;
            }
        }

        // gc hooks for IGCObject_DArray

        std::size_t
        DArray::shallow_size() const noexcept
        {
            return sizeof(DArray);
        }

        DArray *
        DArray::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            DArray * copy = (DArray *)mm.alloc_copy((std::byte *)this);

            if (copy) {
                copy->capacity_ = capacity_;
                copy->size_ = size_;

                constexpr auto c_obj_z = sizeof(obj<AGCObject>);

                /* memcpy sufficient for obj<A,D> */
                ::memcpy((void*)&(copy->elts_[0]), (void*)&(elts_[0]), capacity_ * c_obj_z);
            }

            return copy;
        }

        std::size_t
        DArray::forward_children(obj<ACollector> gc) noexcept
        {
            for (size_type i = 0; i < size_; ++i) {
                obj<AGCObject> & elt = elts_[i];

                gc.forward_inplace(elt.iface(), (void **)&(elt.data_));
            }

            return shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/
