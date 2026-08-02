/** @file quoted_char_ostream.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  operator<<(std::ostream&, quoted_char): render a quoted_char to an ostream.
 *
 *  A migration bridge, mirroring tag_ostream.hpp: the legacy
 *  @c os << quoted_char(ch) idiom appears in ostream-based diagnostics, and
 *  this lets such a site move to xo::pp::quoted_char without being rewritten.
 *  Routes through a FlatSink wrapping @p os, reusing Prettifier<quoted_char>.
 *
 *  Segregated from quoted_char.hpp (which stays free of <ostream>, like
 *  tag.hpp vs tag_ostream.hpp).  Unlike tag_ostream.hpp this does not need
 *  pretty_ostream.hpp: a quoted_char always bottoms out in put_with_escape(),
 *  so the operator<< fallback in pretty() is never instantiated.
 **/

#pragma once

#include "FlatSink.hpp"
#include "pretty.hpp" /* PpSink::pp */
#include "quoted_char.hpp"

namespace xo::pp {
    inline std::ostream &
    operator<<(std::ostream & os, const quoted_char & x) {
        FlatSink sink(os);

        sink.pp(x);

        return os;
    }
} /*namespace xo::pp*/

/* end quoted_char_ostream.hpp */
