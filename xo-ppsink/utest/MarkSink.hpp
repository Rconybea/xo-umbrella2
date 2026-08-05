/** @file MarkSink.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Test-only PpSink recording the token stream, so a test can assert on the
 *  structure FlatSink throws away (FlatSink's begin/split/end are no-ops).
 *
 *  ppsink ships no line-breaking sink -- PrettySink lives in xo-indentlog2,
 *  one level up -- so the token stream is the only way to pin layout structure
 *  from here.  That structure is exactly what a custom printer gets wrong: a
 *  missing split, or an indent passed to both begin() and split(), produces
 *  output that looks right until it has to wrap.
 *
 *    "<G>"      begin()          "<G n>"   begin(n)
 *    "<S s,o>"  split(s,o)       "<N o>"   newline(o)
 *    "</G>"     end()
 **/

#pragma once

#include <xo/ppsink/PpSink.hpp>
#include <ostream>

namespace ut {
    class MarkSink final : public xo::pp::PpSink {
    public:
        explicit MarkSink(std::ostream & os) : os_{os} {}

        using PpSink::split;
        using PpSink::newline;

        PpSink & put(std::string_view s) override { os_ << s; return *this; }
        PpSink & put_with_escape(std::string_view s, bool quote_flag) override {
            if (quote_flag) os_ << '"';
            os_ << s;
            if (quote_flag) os_ << '"';
            return *this;
        }
        PpSink & begin() override { os_ << "<G>"; return *this; }
        PpSink & begin(std::int32_t offset) override {
            os_ << "<G " << offset << ">"; return *this;
        }
        PpSink & split(std::uint32_t spaces, std::int32_t offset) override {
            os_ << "<S " << spaces << "," << offset << ">"; return *this;
        }
        PpSink & newline(std::int32_t offset) override {
            os_ << "<N " << offset << ">"; return *this;
        }
        PpSink & end() override { os_ << "</G>"; return *this; }
        xo::pp::PpSinkInserter stream_open(std::uint32_t) override {
            return xo::pp::PpSinkInserter(this, &os_);
        }
        void stream_commit() override {}

    private:
        std::ostream & os_;
    };
} /*namespace ut*/

/* end MarkSink.hpp */
