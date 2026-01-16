/** @file DString.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DString.hpp"
#include <xo/indentlog/print/pretty.hpp>
#include <algorithm>
#include <cstring>

namespace xo {
    using xo::facet::typeseq;
    using xo::print::ppdetail_atomic;

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

                assert(result);

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
        DString::from_view(obj<AAllocator> mm,
                           std::string_view sv)
        {
            size_type len = sv.size();
            size_type cap = len + 1;

            void * mem = mm.alloc(typeseq::id<DString>(),
                                  sizeof(DString) + cap);

            DString * result = new (mem) DString();
            result->capacity_ = cap;
            result->size_ = len;
            std::memcpy(result->chars_, sv.data(), len);
            result->chars_[len] = '\0';

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

        int
        DString::compare(const DString & lhs, const DString & rhs) noexcept
        {
            return ::strcmp(lhs.chars_, rhs.chars_);
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

        bool
        DString::pretty(const ppindentinfo & ppii) const
        {
            return ppdetail_atomic<const char *>::print_pretty(ppii, &(chars_[0]));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DString.cpp */
