/** @file CVector.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/IAlloc.hpp"
#include <cstdint>

namespace xo {
    namespace scm {
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

            friend class LocalEnv;
            friend class VsmStackFrame;

        private:
            /** number of elements in @ref v_ **/
            std::size_t n_ = 0;
            /** contiguous array of pointers **/
            ElementType * v_ = nullptr;
        };


    } /*namespace scm*/
} /*namespace xo*/

/* end CVector.hpp */
