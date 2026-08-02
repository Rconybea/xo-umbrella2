/** @file PpToken.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "print/PpToken.hpp"
#include "xo/arena/padding.hpp"
#include <cassert>
#include <cstring>
#include <memory>

namespace xo {
    using xo::mm::padding;
    using std::uint32_t;
    using std::int32_t;

    namespace pp {
        PpToken::PpToken(PpTokenFlags tk_type,
                         int32_t tk_viz_len,
                         int32_t tk_len,
                         int32_t tk_offset)
            : tk_flags_{tk_type},
              tk_viz_len_{tk_viz_len},
              tk_len_{tk_len},
              tk_offset_{tk_offset}
        {
        }

        uint32_t
        PpToken::alloc_size() const
        {
            if ((tk_flags_ & k_type_mask) == k_string) {
                return ((PpStringToken *)this)->alloc_size();
            } else {
                return sizeof(PpToken);
            }
        }

        PpStringToken::PpStringToken(int32_t tk_viz_len,
                                     uint32_t tk_len,
                                     uint32_t tk_mem,
                                     const char * tk_chars)
            : PpToken(k_string, tk_viz_len, tk_len),
              tk_mem_{tk_mem}
        {
            assert(tk_len <= tk_mem);

            ::memcpy(tk_chars_, tk_chars, tk_len);

            if (tk_len < tk_mem) {
                // Not essential.  For intelligibility if inspecting memory.

                *(tk_chars_ + tk_len) = '\0';
                //::memset(tk_chars_ + tk_len, '\0', tk_mem - tk_len);
            }
        }

        uint32_t
        PpStringToken::alloc_size(uint32_t tk_size)
        {
            // z0: size before padding
            uint32_t z0 = sizeof(PpStringToken) + tk_size;

            return padding::with_padding(z0, sizeof(uint32_t));
        }

    } /*namespace pp*/
} /*namespace xo*/

/* end PpToken.cpp */
