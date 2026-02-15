/** @file dp.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "Allocator.hpp"
#include <xo/facet/obj.hpp>
#include <cassert>

namespace xo {
    namespace mm {

        /** unimorphic "data pointer" with known representation and owned memory.
         *  runs dtor *but not delete*. Does not store allocator!
         *
         *  Compare with abox<AFacet,DRepr>
         **/
        template <typename DRepr>
        struct dp {
            dp() = default;

            /** dp takes ownership of data @p ptr;
             *  will run dtor when dp goes out of scope.
             *
             *  Note this is not useful when DRepr=DVariablePlaceholder
             **/
            explicit dp(DRepr * ptr) : ptr_{ptr} {}

            /** (copy ctor not supported -- ownership is unique) **/
            dp(const dp & other) = delete;

            /** Move constructor **/
            dp(dp<DRepr> && other)
            {
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }

            /** allocates for sizeof(DRepr), so DRepr must not use flexible array **/
            template <typename... Args>
            static dp make(obj<AAllocator> alloc, Args&&... args) {
                void * mem = alloc.alloc_for<DRepr>();

                if (mem) {
                    DRepr * data = ::new (mem) DRepr(std::forward<Args>(args)...);
                    assert(data);

                    return dp(data);
                } else {
                    assert(false);

                    return dp();
                }
            }

            dp<DRepr> & operator=(const dp<DRepr> & x) = delete;

            /** move assignment **/
            dp<DRepr> & operator=(dp<DRepr> && x) {
                ptr_ = x.ptr_;
                x.ptr_ = nullptr;
            }

            // --------------------------------

            DRepr * data() const noexcept { return ptr_; }

            operator bool() const noexcept { return ptr_ != nullptr; }

            DRepr * operator->() const noexcept { return ptr_; }
            DRepr & operator*() const noexcept { return *ptr_; }

#ifdef NOT_YET
            /** explicit conversion to obj<AFacet,DRepr> **/
            obj<AFacet, DRepr> to_op() const noexcept {
                return obj<AFacet, DRepr>(this->iface(), this->data());
            }
#endif

#ifdef NOT_YET
            /** Take ownership from unowned object **/
            template <typename AFacet>
            dp & adopt(const obj<AFacet, DRepr> & other)
            requires (std::is_same_v<DRepr, DVariantPlaceholder>
                      || std::is_same_v<DRepr, DOther>)
            {
                /* replace .iface_ along w/ .data_ */
                this->from_obj(other);

                return *this;
            }
#endif

            ~dp() {
                if (ptr_) {
                    ptr_->~DRepr();
                }
            }

        private:
            DRepr * ptr_ = nullptr;
        };
    } /*namespace mm*/

    using mm::dp;
} /*namespace xo*/

/* end dp.hpp */
