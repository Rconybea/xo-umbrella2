/** @file quoted_ostream.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  operator<<(std::ostream&, quot_impl): render a quot()/unq() to an ostream.
 *
 *  A migration bridge, mirroring tag_ostream.hpp: the legacy
 *  @c os << quot(s) idiom is common in ostream-based display() / operator<<
 *  methods, and this lets such a site move to xo::pp::quot without being
 *  rewritten.  Routes through a FlatSink wrapping @p os, reusing
 *  Prettifier<quot_impl>.
 *
 *  Not needed to use quot()/unq() as a tag value -- @c os << xtag("k", unq(s))
 *  reaches Prettifier<quot_impl> through the tag machinery, so tag_ostream.hpp
 *  plus quoted.hpp is enough there.
 *
 *  Segregated from quoted.hpp (which stays free of <ostream>, like tag.hpp vs
 *  tag_ostream.hpp).  Does not need pretty_ostream.hpp: a quot_impl always
 *  bottoms out in put_with_escape(), so pretty()'s operator<< fallback is never
 *  instantiated.
 **/

#pragma once

#include "quoted.hpp"
#include "FlatSink.hpp"
#include "pretty.hpp"   /* PpSink::pp */

namespace xo::pp {
    template <quotestyle QuoteStyle>
    inline std::ostream &
    operator<<(std::ostream & os, const quot_impl<QuoteStyle> & x) {
        FlatSink sink(os);

        sink.pp(x);

        return os;
    }
} /*namespace xo::pp*/

/* end quoted_ostream.hpp */
