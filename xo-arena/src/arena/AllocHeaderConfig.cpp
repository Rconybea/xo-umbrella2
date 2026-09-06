/** @file AllocHeaderConfig.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "AllocHeaderConfig.hpp"
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/hex.hpp>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;
    using xo::pp::hex;
    using xo::pp::hexprefix;

    namespace mm {
        void
        AllocHeaderConfig::pretty(PpSink & sink) const
        {
            /* NB the bit-width members are uint8_t.  xo::pp deliberately
             * excludes unsigned char from pp_number_integral (see the
             * concept in Prettifier.hpp) because it cannot tell a small
             * number from a character, so these must be promoted first --
             * and via named locals, since field() captures by reference.
             */
            std::uint32_t tseq_bits = tseq_bits_;
            std::uint32_t age_bits = age_bits_;
            std::uint32_t size_bits = size_bits_;
            /* guard_byte_ is a bit PATTERN, not a count -- render it in hex,
             * the way the header spells it.  Qualified, because every
             * neighbouring field here is decimal: a bare "fd" would not say
             * which radix it is in.
             */
            hex guard_byte{guard_byte_, hexprefix::qualified};

            /* guard_* omitted unless enabled: guard bytes are a debugging
             * feature, and printing "guard_z=0" on every arena config is noise
             */
            sink.pretty_struct("AllocHeaderConfig",
                               field("tseq_bits", tseq_bits),
                               field("age_bits", age_bits),
                               field("size_bits", size_bits),
                               field("guard_z", guard_z_, guard_z_ > 0),
                               field("guard_byte", guard_byte, guard_z_ > 0));
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocHeaderConfig.cpp */
