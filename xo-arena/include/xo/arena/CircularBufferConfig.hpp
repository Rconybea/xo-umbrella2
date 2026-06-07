/** @file CircularBufferConfig.hpp
*
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <string>

namespace xo {
    namespace mm {
        /** @class CircularBufferConfig
         *
         *  @brief configuration for a @ref DCircularBuffer instance
         **/
        struct CircularBufferConfig {
            /** @defgroup mm-circularbufferconfig-instance-vars CircularBufferConfig members **/
            ///@{

            /** optional name, for diagnostics **/
            std::string name_;
            /** hard maximum buffer size = reserved virtual memory.
             *  However actual max will be this value rounded up to at least page size.
             *  Buffer will generally map much less than this amount of memory
             **/
            std::size_t max_capacity_ = 0;
            /** hugepage size -- using huge pages relieves some TLB pressure,
             *  at expense of inefficient memory consumption for (up to two)
             *  partially used superpages.
             **/
            std::size_t hugepage_z_ = 2 * 1024 * 1024;
            /** Threshold 'move efficeincy' = (move_distance / move_qty)
             *  applies to moving unread input to the beginning of mapped range,
             *  when not prevented by pinned ranges.
             *
             *  Higher numbers reduce cpu consumption but increase memory consumption
             *  Reciprocal loose ceiling on relative effort that may be spent on
             *  moving fractional input
             **/
            float threshold_move_efficiency_ = 50.0;
            /** lower bound for hard maximum number of capture spans.
             *
             *  Expected use case is to track spans that are currently referenced
             *  (rather than copied) from outside a DCircularBuffer instance.
             *  Circular buffer will not unmap or overwrite memory for such spans.
             *
             *  Expect to generally release captured spans in the same order they
             *  were captured. Out of order release is supported, but cost
             *  of out-of-order release grows
             *  like O(r) for r remembered spans.
             *
             *  A typical parser will need spans to remember one line of input
             **/
            std::size_t max_captured_span_ = 0;
            /** true to enable debug logging **/
            bool debug_flag_ = false;

            ///@}
        };
    } /*namespace mm*/
} /*namespace xo*/

/* end CircularBufferConfig.hpp */
