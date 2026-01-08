/** @file ControlGroupo
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArenaHashMapUtil.hpp"
#include <array>
#include <cstdint>
#include <cstring>

namespace xo {
    namespace mm {
        namespace detail {
            /** @brief 16x 8-bit control bytes.
             *
             *  Support optimization using SIMD operations
             **/
            struct Group {
                std::array<uint8_t, DArenaHashMapUtil::c_group_size> ctrl_;

                explicit Group(uint8_t * lo) {
                    ::memcpy(ctrl_.data(), lo, DArenaHashMapUtil::c_group_size);
                }

                /** find all exact matches in ctrl_[0..15] for @p h2.
                 *  for each match set corresponding bit in return value.
                 *  Bits {0x1, 0x2, 0x4, ...} set iff exact match on
                 *  {ctrl_[0], ctrl_[1], ctrl_2[], ...} respectively
                 **/
                uint16_t all_matches(uint8_t h2) const {
                    uint16_t retval = 0;
                    uint16_t bit = 1;
                    for (auto xi : ctrl_) {
                        if (xi == h2)
                            retval |= bit;
                        bit = bit << 1;
                    }

                    return retval;
                }

                /** find all empty sentinels in ctrl_[0..15].
                 *  for each empty, set corresponding bit in return value.
                 *  Bits {0x1, 0x2, 0x4, ...} set iff empty spot
                 *  {ctrl_[0], ctrl_[1], ctrl_[2], ...} respectively
                 **/
                uint16_t empty_matches() const {
                    uint16_t retval = 0;
                    uint16_t bit = 1;

                    for (auto xi : ctrl_) {
                        if (xi == DArenaHashMapUtil::c_empty_slot)
                            retval |= bit;
                        bit = bit << 1;
                    }

                    return retval;
                }

#ifdef NOT_YET
                __m128i ctrl;  // 16 bytes loaded via SSE2

                // Find all slots matching h2
                uint16_t Match(uint8_t h2) const {
                    __m128i pattern = _mm_set1_epi8(h2);
                    __m128i result = _mm_cmpeq_epi8(ctrl, pattern);
                    return _mm_movemask_epi8(result);  // 16-bit mask
                }

                // Find all empty slots (0xFF)
                uint16_t MatchEmpty() const {
                    return _mm_movemask_epi8(_mm_cmpeq_epi8(ctrl, _mm_set1_epi8(0xFF)));
                }
#endif
            };
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end ControlGroup.hpp */
