/* file LinearAlloc.hpp
 *
 * author: Roland Conybeare, Jul 2025
 */

#pragma once

#include "IAlloc.hpp"

namespace xo {
    namespace gc {
        /** @class LinearAlloc
         *  @brief Bump allocator with fixed capacity
         *
         *  @text
         *
         *  before @ref release_redline_memory
         *
         *    <-----allocated----> <-free-> <-reserved->
         *    XXXXXXXXXXXXXXXXXXXX______________________
         *    ^                   ^        ^            ^
         *    lo                  free     redline      hi
         *                                 limit
         *
         *  after @ref release_redline_memory
         *
         *    <-----allocated----> <--------free------->
         *    XXXXXXXXXXXXXXXXXXXX______________________
         *    ^                   ^                     ^
         *    lo                  free                  hi
         *                                              limit
         *  @endtext
         *
         *  TODO: rename to ArenaAlloc
         **/
        class LinearAlloc : public IAlloc {
        public:
            ~LinearAlloc();

            /** create allocator with capacity @p z,
             *  with reserved capacity @p redline_z.
             **/
            static up<LinearAlloc> make(std::size_t redline_z, std::size_t z);

            std::uint8_t * free_ptr() const { return free_ptr_; }
            void       set_free_ptr(std::uint8_t * x);

            // inherited from IAlloc...

            virtual std::size_t size() const override;
            virtual std::size_t available() const override;
            virtual std::size_t allocated() const override;
            virtual bool is_before_checkpoint(const std::uint8_t * x) const override;
            virtual std::size_t before_checkpoint() const override;
            virtual std::size_t after_checkpoint() const override;

            virtual void clear() override;
            virtual void checkpoint() override;
            virtual std::uint8_t * alloc(std::size_t z) override;


        private:
            LinearAlloc(std::size_t rz, std::size_t z);

        private:
            /**
             *  Invariants:
             *  - @ref free_ always a multiple of word size (assumed to be sizeof(void*))
             **/

            /** allocator owns memory in range [@ref lo_, @ref hi_) **/
            std::uint8_t * lo_ = nullptr;
            /** checkpoint (for GC support); divides objects into
             *  older (addresses below checkpoint)
             *  and younger (addresses above checkpoint)
             **/
            std::uint8_t * checkpoint_;
            /** free pointer. memory in range [@ref free_, @ref limit_) available **/
            std::uint8_t * free_ptr_ = nullptr;
            /** soft limit: end of released memory **/
            std::uint8_t * limit_ = nullptr;
            /** hard limit: end of allocated memory **/
            std::uint8_t * hi_ = nullptr;
        };

    } /*namespace gc*/
} /*namespace xo*/


/* end LinearAlloc.hpp */
