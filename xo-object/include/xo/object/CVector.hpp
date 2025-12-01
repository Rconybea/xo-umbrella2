/** @file CVector.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/allocutil/IAlloc.hpp"
#include <cstdint>

namespace xo {
    namespace obj {
        /** gc-only vector.
         *  Used in both LocalEnv and VsmStackFrame
         **/
        template <typename ElementType>
        class CVector {
        public:
            using value_type = ElementType;

        public:
            CVector(gc::IAlloc * mm, std::size_t n)
                : n_{n}, v_{nullptr}
                {
                    if (n_ > 0) {
                        std::byte * mem = mm->alloc(n_ * sizeof(ElementType));
                        this->v_ = new (mem) ElementType[n];
                    }
                }

            std::size_t size() const { return n_; }

            ElementType operator[](std::size_t i) const { return v_[i]; }
            ElementType & operator[](std::size_t i) { return v_[i]; }

        public:
            /** number of elements in @ref v_ **/
            std::size_t n_ = 0;
            /** contiguous array of pointers **/
            ElementType * v_ = nullptr;
        };
    } /*namespace obj*/
} /*namespace xo*/

/* end CVector.hpp */
