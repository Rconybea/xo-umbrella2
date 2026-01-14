/** @file DString.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DString.hpp"
#include <algorithm>
#include <cstring>

namespace xo {
    using xo::facet::typeseq;

    namespace scm {
        DString *
        DString::empty(obj<AAllocator> mm,
                       size_type cap)
        {
            assert(cap > 0);

            DString * result = nullptr;

            if (cap > 0) {
                void * mem = mm.alloc(typeseq::id<DString>(),
                                      sizeof(DString) + cap);

                result = new (mem) DString();
                result->capacity_ = cap;
                result->size_ = 0;
                if (cap > 0) {
                    result->chars_[0] = '\0';
                }
            }

            return result;
        }

        DString *
        DString::from_cstr(obj<AAllocator> mm,
                           const char * cstr)
        {
            size_type len = std::strlen(cstr);
            size_type cap = len + 1;

            void * mem = mm.alloc(typeseq::id<DString>(),
                                  sizeof(DString) + cap);

            DString * result = new (mem) DString();
            result->capacity_ = cap;
            result->size_ = len;
            std::memcpy(result->chars_, cstr, cap);

            return result;
        }

        DString *
        DString::clone(obj<AAllocator> mm, const DString * src)
        {
            size_type cap = src->capacity_;

            void * mem = mm.alloc(typeseq::id<DString>(),
                                  sizeof(DString) + cap);

            DString * result = new (mem) DString();
            result->capacity_ = cap;
            result->size_ = src->size_;
            std::memcpy(result->chars_, src->chars_, cap);

            return result;
        }

        DString &
        DString::assign(const DString & other)
        {
            size_type n = std::min(other.size_, capacity_ - 1);
            std::memcpy(chars_, other.chars_, n);
            chars_[n] = '\0';
            size_ = n;

            return *this;
        }

        auto
        DString::fixup_size() noexcept -> size_type
        {
            this->chars_[capacity_ - 1] = '\0';
            this->size_ = ::strlen(chars_);
            return this->size_;
        }

        auto
        DString::shallow_size() const noexcept -> size_type
        {
            return sizeof(DString) + capacity_;
        }

        DString *
        DString::shallow_copy(obj<AAllocator> mm) const noexcept
        {
            DString * copy = (DString *)mm.alloc_copy((std::byte *)this);

            if (copy) {
                copy->capacity_ = capacity_;
                copy->size_ = size_;
                ::memcpy(copy->chars_, chars_, capacity_);
            }

            return copy;
        }

        auto
        DString::forward_children(obj<ACollector>) noexcept -> size_type
        {
            return shallow_size();
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DString.cpp */
