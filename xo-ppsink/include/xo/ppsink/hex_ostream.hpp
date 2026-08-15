/** @file hex_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, hex_view): render a hex_view to an ostream.
 *
 *  A migration bridge, mirroring tag_ostream.hpp / quoted_ostream.hpp.  The
 *  legacy idiom
 *    std::cerr << hex_view(s.c_str(), s.c_str() + s.size(), true) << std::endl;
 *  appears (usually commented out, as a prompt for the next time someone needs
 *  to inspect that memory) in ostream-based debugging code across xo.  This
 *  header lets such a site move to xo::pp::hex_view without being rewritten.
 *
 *  Routes through a FlatSink wrapping @p os, reusing Prettifier<hex_view>.
 *  FlatSink renders splits as plain spaces, so a hex_view streamed to an
 *  ostream comes out flat regardless of length -- byte-for-byte what legacy
 *  produced, with no 16-byte wrapping.  Wrapping needs a sink that tracks a
 *  margin (PrettySink).
 *
 *  Not needed to use a hex_view as a tag value -- os << xtag("buf", hex_view(s))
 *  reaches Prettifier<hex_view> through the tag machinery, so tag_ostream.hpp
 *  plus hex.hpp is enough there.
 *
 *  Segregated from hex.hpp (which stays free of <ostream>, like tag.hpp vs
 *  tag_ostream.hpp).  Does not need pretty_ostream.hpp: a hex_view always
 *  bottoms out in put(), so pretty()'s operator<< fallback is never
 *  instantiated.
 **/

#pragma once

#include "FlatSink.hpp"
#include "hex.hpp"
#include "pretty.hpp" /* PpSink::pp */

namespace xo::pp {
    inline std::ostream &
    operator<<(std::ostream & os, const hex_view & x) {
        FlatSink sink(os.rdbuf());

        sink.pp(x);

        return os;
    }
} /*namespace xo::pp*/

/* end hex_ostream.hpp */
