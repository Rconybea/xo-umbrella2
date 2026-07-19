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

            bool is_string() const { return (tk_flags_ & k_type_mask) == k_string; }
            bool is_begin() const { return (tk_flags_ & k_type_mask) == k_begin; }
            bool size_established() const {
                auto tk_type = tk_flags_ & k_type_mask;
                if (tk_type == k_begin)
                    return (tk_flags_ & k_size_established) != 0;

                // {string, split, end} token size is always established
                return true;
            }

            uint32_t alloc_size() const;

            void set_fits_flag(bool x) {
                uint32_t fits = (x ? k_fits : 0);
                tk_flags_ = (tk_flags_ & ~k_fits) | fits;
            }

            void establish_size(int32_t tk_viz_len, int32_t tk_len) {
                tk_flags_ = (tk_flags_ & ~k_size_established) | k_size_established;
                tk_viz_len_ = tk_viz_len;
                tk_len_ = tk_len;
            }

        protected:
            PpToken(PpTokenFlags tk_flags,
                    int32_t tk_viz_len,
                    int32_t tk_len);

            friend class PpState;

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

            /** padded size in bytes for a PpStringToken instance
             *  with space for tk_size characters.
             *  Return value includes sizeof(PpStringToken) itself;
             *  @p tk_size does not.
             **/
            static uint32_t alloc_size(uint32_t tk_size);

            uint32_t tk_mem() const { return tk_mem_; }
            Span span() const { return Span(&(tk_chars_[0]),
                                            &(tk_chars_[this->tk_len()])); }
            Span mem_span() const { return Span(&(tk_chars_[0]),
                                                &(tk_chars_[this->tk_mem()])); }
            uint32_t alloc_size() const { return sizeof(PpStringToken) + tk_mem_; }

            /** finalize string token in-place
             *  @p tk_viz_len  token visible length
             *  @p tk_len      token length (including non-visible chars)
             *  @p tk_mem      Space actually used by tk_chars.
             *                 (will round up for alignment reasons)
             **/
            void finalize_inplace(uint32_t tk_viz_len, uint32_t tk_len, uint32_t tk_mem) {
                this->establish_size(tk_viz_len, tk_len);
                this->tk_mem_ = tk_mem;
            }

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
