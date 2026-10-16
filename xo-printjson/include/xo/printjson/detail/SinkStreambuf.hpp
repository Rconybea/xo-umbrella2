/** @file SinkStreambuf.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  SinkStreambuf: a streambuf whose writes go to a PpSink, via put().
 *
 *  Temporary: it backs JsonPrintState::p_os() while printers move from the
 *  ostream to the sink one at a time, then goes, with p_os().  See
 *  .xo-backlog/xo-printjson/issues/01, "Decision".
 *
 *  Unbuffered on purpose: every write reaches the sink at once, so ostream
 *  writes and direct sink calls interleave in program order.  A put area
 *  would hold bytes back past a later direct sink call.
 **/

#pragma once

#include <xo/ppsink/PpSink.hpp>
#include <streambuf>
#include <string_view>

namespace xo {
    namespace json {
        namespace detail {
            class SinkStreambuf : public std::streambuf {
            public:
                /** writes go to @c *p_sink -- read at each write, so the
                 *  owner may redirect them by changing what it points to
                 **/
                explicit SinkStreambuf(xo::pp::PpSink * const * p_sink) : p_sink_{p_sink} {}

            protected:
                virtual int_type overflow(int_type c) override {
                    if (!traits_type::eq_int_type(c, traits_type::eof())) {
                        char ch = traits_type::to_char_type(c);

                        (*p_sink_)->put(std::string_view(&ch, 1));
                    }

                    return traits_type::not_eof(c);
                }

                virtual std::streamsize xsputn(const char * s, std::streamsize n) override {
                    (*p_sink_)->put(std::string_view(s, n));

                    return n;
                }

            private:
                /** where writes go **/
                xo::pp::PpSink * const * p_sink_ = nullptr;
            };
        } /*namespace detail*/
    } /*namespace json*/
} /*namespace xo*/

/* end SinkStreambuf.hpp */
