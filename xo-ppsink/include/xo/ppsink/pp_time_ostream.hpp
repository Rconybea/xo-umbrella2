/** @file pp_time_ostream.hpp
 *
 *  @author Roland Conybeare, Aug 2026
 *
 *  operator<<(std::ostream&, ..) for the pp_time.hpp value wrappers.
 *
 *  A migration bridge, mirroring tag_ostream.hpp / quoted_ostream.hpp: the
 *  legacy idiom
 *    os << "\"" << iso8601(t0) << "\"";
 *  appears in ostream-based code, and this lets such a site move to
 *  xo::pp::iso8601 without being rewritten.  Routes through a FlatSink
 *  wrapping @p os, reusing each wrapper's Prettifier.
 *
 *  Segregated from pp_time.hpp, which stays free of <ostream>.
 *
 *  NB there is deliberately NO operator<< here for bare xo::time::utc_nanos or
 *  xo::time::nanos.  Those are std::chrono types, so an unqualified `os << t`
 *  resolves by ADL into namespace std::chrono and would never find an overload
 *  declared in xo::pp.  Legacy xo-indentlog "solved" that by declaring its
 *  inserters INSIDE namespace std::chrono -- undefined behaviour, and it
 *  silently governed how every time_point in the program printed.
 *
 *  The supported routes for a bare time, both of which use
 *  Prettifier<utc_nanos> from pp_time.hpp:
 *  @code
 *    xo::pp::pp_to_stream(os, t0);          // ostream in hand
 *    sink.pp(t0);                           // sink in hand
 *    os << xo::pp::ymd_hms_usec(t0);        // or say the format explicitly
 *  @endcode
 *
 *  Left to C++20's own inserter, `os << t0` yields
 *  "2022-09-26 09:30:00.123456000" -- which embeds a space, so it does not
 *  read back as a single token.  Every format in pp_time.hpp is space-free
 *  for exactly that reason.
 **/

#pragma once

#include "FlatSink.hpp"
#include "pp_time.hpp"
#include "pretty.hpp" /* PpSink::pp */
#include <ostream>

namespace xo::pp {
    inline std::ostream &
    operator<<(std::ostream & os, iso8601 x) {
        FlatSink sink(os);
        sink.pp(x);
        return os;
    }

    inline std::ostream &
    operator<<(std::ostream & os, hms_msec x) {
        FlatSink sink(os);
        sink.pp(x);
        return os;
    }

    inline std::ostream &
    operator<<(std::ostream & os, hms_usec x) {
        FlatSink sink(os);
        sink.pp(x);
        return os;
    }

    inline std::ostream &
    operator<<(std::ostream & os, ymd_hms_usec x) {
        FlatSink sink(os);
        sink.pp(x);
        return os;
    }
} /*namespace xo::pp*/

/* end pp_time_ostream.hpp */
