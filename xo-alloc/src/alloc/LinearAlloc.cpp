/* file LinearAlloc.cpp
 *
 * author: Roland Conybeare
 */

#include "LinearAlloc.hpp"
#include "xo/indentlog/print/tag.hpp"
#include <cassert>

namespace xo {
    namespace gc {
        LinearAlloc::LinearAlloc(std::size_t rz, std::size_t z)
        {
            this->lo_         = (new std::uint8_t [rz + z]);
            this->checkpoint_ = lo_;
            this->free_ptr_   = lo_;
            this->limit_      = lo_ + z;
            this->hi_         = limit_ + rz;

            if (!lo_) {
                throw std::runtime_error(tostr("LinearAlloc: allocation failed",
                                               xtag("size", rz + z)));
            }
        }

        LinearAlloc::~LinearAlloc()
        {
            delete [] this->lo_;

            // hygiene..

            this->lo_ = nullptr;
            this->checkpoint_ = nullptr;
            this->free_ptr_ = nullptr;
            this->limit_ = nullptr;
            this->hi_ = nullptr;
        }

        up<LinearAlloc>
        LinearAlloc::make(std::size_t rz, std::size_t z)
        {
            return up<LinearAlloc>(new LinearAlloc(rz, z));
        }

        void
        LinearAlloc::set_free_ptr(std::uint8_t * x)
        {
            assert(lo_ <= x);
            assert(x < limit_);

            if (lo_ <= x && x < limit_) {
                this->free_ptr_ = x;
            } else {
                throw std::runtime_error(tostr("LinearAllog::set_free_ptr(x): expected lo <= x < limit",
                                               xtag("lo", lo_), xtag("x", x), xtag("limit", limit_)));
            }
        }

        std::size_t
        LinearAlloc::size() const {
            return limit_ - lo_;
        }

        std::size_t
        LinearAlloc::available() const {
            return limit_ - free_ptr_;
        }

        std::size_t
        LinearAlloc::allocated() const {
            return free_ptr_ - lo_;
        }

        bool
        LinearAlloc::is_before_checkpoint(const std::uint8_t * x) const {
            return (lo_ <= x) && (x < checkpoint_);
        }

        std::size_t
        LinearAlloc::before_checkpoint() const
        {
            return checkpoint_ - lo_;
        }

        std::size_t
        LinearAlloc::after_checkpoint() const
        {
            return free_ptr_ - checkpoint_;
        }

        void
        LinearAlloc::clear()
        {
            this->checkpoint_ = lo_;
            this->free_ptr_ = lo_;
            this->limit_ = lo_;
        }

        void
        LinearAlloc::checkpoint()
        {
            this->checkpoint_ = this->free_ptr_;
        }

        std::uint8_t *
        LinearAlloc::alloc(std::size_t z)
        {
            /* word size for alignment */
            constexpr uint32_t c_bpw = sizeof(void*);

            std::uintptr_t free_u64 = reinterpret_cast<std::uintptr_t>(free_ptr_);

            assert(free_u64 % c_bpw == 0ul);

            /* round up to multiple of c_bpw */
            std::uint32_t dz = (c_bpw - (z % c_bpw));
            z += dz;

            assert(z % c_bpw == 0ul);

            std::uint8_t * retval = this->free_ptr_;

            this->free_ptr_ += z;

            if (free_ptr_ > limit_) {
                return nullptr;
            }

            return retval;
        }
    } /*namespace gc*/
} /*namespace xo*/


/* end LinearAlloc.cpp */
