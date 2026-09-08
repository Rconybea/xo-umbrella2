/** @file MemorySizeInfo.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "MemorySizeInfo.hpp"
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;

    namespace mm {
        void
        MemorySizeDetail::pretty(PpSink & sink) const
        {
            /* tseq_ is a class with no numeric conversion; seqno() is the
             * printable form, and -1 marks the totals row / an empty slot
             */
            std::int32_t tseq = tseq_.seqno();

            sink.pretty_struct("MemorySizeDetail",
                               field("tseq", tseq),
                               field("n_alloc", n_alloc_),
                               field("z_alloc", z_alloc_));
        }

        std::size_t
        MemorySizeInfo::n_detail() const noexcept
        {
            std::size_t retval = 0;

            if (detail_) {
                for (const auto & detail : *detail_) {
                    if (detail.n_alloc_)
                        ++retval;
                }
            }

            return retval;
        }

        void
        MemorySizeInfo::pretty(PpSink & sink) const
        {
            /* allocated_ omitted when it agrees with used_ -- they differ only
             * for a pool that wastes space (empty hash slots, say), and that
             * difference is the reason to look.
             *
             * The histogram itself is not rendered: it is up to 32 rows, and a
             * report is typically one line in a list of pools.  Its size is
             * reported instead, so a reader knows there is something to ask
             * for; iterate detail_ to see it.
             */
            std::size_t n_detail = this->n_detail();

            sink.pretty_struct("MemorySizeInfo",
                               field("name", resource_name_, !resource_name_.empty()),
                               field("used", used_),
                               field("allocated", allocated_, allocated_ != used_),
                               field("committed", committed_),
                               field("reserved", reserved_),
                               field("lo", lo_, lo_ != nullptr),
                               field("hi", hi_, hi_ != nullptr),
                               field("n_detail", n_detail, n_detail > 0));
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end MemorySizeInfo.cpp */
