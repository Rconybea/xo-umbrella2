/** @file PpToken.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#pragma once

#include "PpTokenType.hpp"
#include <xo/arena/span.hpp>
#include <cstdint>

namespace xo {
    namespace print {
        class PpToken {
        public:
            using uint32_t = std::uint32_t;

        public:
            uint32_t tk_flags() const { return tk_flags_; }
            int32_t tk_viz_len() const { return tk_viz_len_; }
            int32_t tk_len() const { return tk_len_; }

            bool has_unknown_size() const { return tk_viz_len_ == 0; }
            uint32_t alloc_size() const;

            void set_fits_flag(bool x) {
                uint32_t fits = (x ? k_fits : 0);
                tk_flags_ = (tk_flags_ & ~k_fits) | fits;
            }

        protected:
            PpToken(PpTokenFlags tk_flags,
                    int32_t tk_viz_len,
                    int32_t tk_len);

        private:
            /** token type: k_string|k_begin|k_break|k_end.
             *  fits flag: k_fits
             **/
            uint32_t tk_flags_ = k_nominal;
            /**
             *  -1 :: does not fit -> visible length doesn't matter
             *   0 :: visible length unknown
             *  >0 :: visible token length
             **/
            int32_t tk_viz_len_ = 0;
            /**
             *  -1  :: doesn't matter, doesn't fit.
             *  0   :: token length unknown
             *  +ve :: actual token length (characters)
             **/
            int32_t tk_len_ = 0;
        };

        class PpStringToken : public PpToken {
        public:
            using Span = xo::mm::span<const char>;

        public:
            PpStringToken(int32_t tk_viz_z,
                          uint32_t tk_size,
                          uint32_t tk_mem,
                          const char * tk_chars);

            Span span() const { return Span(&(tk_chars_[0]),
                                            &(tk_chars_[this->tk_len()])); }
            uint32_t alloc_size() const { return sizeof(PpStringToken) + tk_mem_; }

            /** padded size in bytes for a PpStringToken instance
             *  with token size @p tk_size
             **/
            static uint32_t alloc_size(uint32_t tk_size);

        private:
            /** amount of space used by tk_chars_[]. **/
            uint32_t tk_mem_ = 0;
            /** k_string: array of characters to print.
             *  Not used for k_begin|k_break|k_end.
             */
            char tk_chars_[];
        };
    } /*namespace print*/
} /*namespace xo*/

/* end PpToken.hpp */
