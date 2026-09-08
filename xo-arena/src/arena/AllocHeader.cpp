/** @file AllocHeader.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "AllocHeader.hpp"
#include <xo/ppsink/pretty_struct.hpp>

namespace xo {
    using xo::pp::PpSink;
    using xo::pp::field;

    namespace mm {
        void
        AllocHeader::pretty(PpSink & sink) const
        {
            /* repr_ is a packed bit field, not a number: decimal would say
             * nothing.  Rendered through the pointer prettifier, which is
             * xo::pp's 0x-qualified whole-word hex printer -- the `hex` class
             * covers a single byte only.
             */
            const void * repr = reinterpret_cast<const void *>(repr_);

            sink.pretty_struct("AllocHeader", field("repr", repr));
        }
    } /*namespace mm*/
} /*namespace xo*/

/* end AllocHeader.cpp */
