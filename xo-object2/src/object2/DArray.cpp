/** @file DArray.cpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DArray.hpp"

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
    } /*namespace scm*/
} /*namespace xo*/
