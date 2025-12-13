/** @file padding.hpp
 *
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <memory>
#include <cstdint>

namespace xo {
    namespace mm {

        struct padding {
            /** word size for alignment**/
            static constexpr std::size_t c_alloc_alignment = sizeof(std::uintptr_t);

            static inline std::size_t is_aligned(std::size_t n,
                                                 std::size_t align = c_alloc_alignment) {
                return n % align == 0;
            }

            /** how much to add to @p z to get a multiple of
             *  @ref c_alloc_alignment
             **/
            static inline std::size_t alloc_padding(std::size_t z,
                                                    std::size_t align = c_alloc_alignment)
                {

                    /* round up to multiple of c_bpw, but map 0 -> 0
                     * (table assuming c_bpw==8)
                     *
                     *   z%c_bpw   dz
                     *   ------------
                     *         0    0
                     *         1    7
                     *         2    6
                     *        ..   ..
                     *         7    1
                     */
                    std::size_t dz = (align - (z % align)) % align;

                    z += dz;

                    return dz;
                }

            /** @p z rounded up to an exact multiple
             *  of @ref c_alloc_alignment
             **/
            static inline
            std::size_t with_padding(std::size_t z,
                                     std::size_t align = c_alloc_alignment)
                {
                    return z + alloc_padding(z, align);
                }
        };

    } /*namespace mm*/
} /*namespace xo*/

/* end padding.hpp */
