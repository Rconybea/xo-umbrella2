/** @file tag_ostream.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  operator<<(std::ostream&, tag_impl): render an xo::pp tag to an ostream.
 *
 *  A migration bridge.  The legacy `os << xtag("k", v)` idiom -- a tag used as
 *  an ostream inserter -- is pervasive across xo (ostream-based display() /
 *  operator<< methods).  Rewriting every such method into a PpSink method is a
 *  long, incremental job, so this header lets a site switch from the legacy
 *  xo::tag to xo::pp::xtag *without* being rewritten: it routes the tag through
 *  a FlatSink wrapping @p os, reusing Prettifier<tag_impl>.
 *
 *  Segregated from tag.hpp (which stays free of <ostream>, like pretty.hpp vs
 *  pretty_ostream.hpp).  Include this only where you stream a tag to an ostream;
 *  a value with only operator<< (no Prettifier) reaches the pretty() fallback,
 *  which is why the <ostream>-bearing pretty_ostream.hpp is pulled in here.
 **/

#pragma once

#include "FlatSink.hpp"
#include "pretty_ostream.hpp" /* <ostream> + operator<< fallback for tag values */
#include "tag.hpp"

namespace xo::pp {
    /** stream a tag to @p os by rendering it through a FlatSink.
     *
     *  Produces the same text as sink.pp(t): ":name value" (xtag adds a
     *  leading space).  The value renders via pretty() -- Prettifier<Value>
     *  if it has one, else its operator<<.
     **/
    template <bool PrefixSpace, tagstyle TagStyle, typename Name, typename Value>
    inline std::ostream &
    operator<<(std::ostream & os,
               const tag_impl<PrefixSpace, TagStyle, Name, Value> & t) {
        FlatSink sink(os.rdbuf());
        sink.pp(t);
        return os;
    }
} /*namespace xo::pp*/

/* end tag_ostream.hpp */
