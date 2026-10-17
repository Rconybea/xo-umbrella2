/** @file JsonPpSink.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#include "JsonPpSink.hpp"
#include "json_layout.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/quoted.hpp>
#include <charconv>
#include <stdexcept>
#include <string>

namespace xo {
    using xo::pp::escapestyle;
    using xo::pp::quot;
    using xo::pp::tojsonstr;
    using xo::pp::tostr;

    namespace json {
        JsonPpSink::JsonPpSink(PpSink * sink)
            : sink_{sink}
        {
            if (!sink || (sink->style().escape != escapestyle::json)) {
                throw std::runtime_error
                    (tostr("JsonPpSink: sink must escape for json"
                           " (PpStyle::escape == escapestyle::json, e.g. PpStyle::json())",
                           (sink ? "" : "; sink is null")));
            }

            stack_.push_back(Frame{});
        }

        // ----- structure -----

        JsonPpSink &
        JsonPpSink::open_object()
        {
            this->begin_nonkey_value("open_object()");
            this->open_group(frame_kind::object, '{', 1);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::close_object()
        {
            this->close_group(frame_kind::object, '}');

            return *this;
        }

        JsonPpSink &
        JsonPpSink::open_list(std::uint32_t sep_spaces)
        {
            this->begin_nonkey_value("open_list()");
            this->open_group(frame_kind::list, '[', sep_spaces);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::close_list()
        {
            this->close_group(frame_kind::list, ']');

            return *this;
        }

        JsonPpSink &
        JsonPpSink::key(std::string_view k)
        {
            Frame & f = stack_.back();

            if (f.kind_ != frame_kind::object)
                this->misuse("key() outside an object");
            if (f.want_value_)
                this->misuse("key() where the previous key's value is expected");

            if (!f.first_)
                this->separator(1);
            f.first_ = false;
            f.want_value_ = true;

            (*sink_)(quot(k), ": ");

            return *this;
        }

        // ----- values -----

        JsonPpSink &
        JsonPpSink::string(std::string_view s)
        {
            if (this->begin_value())
                return this->key(s);

            sink_->pp(quot(s));

            return *this;
        }

        JsonPpSink &
        JsonPpSink::number_signed(std::int64_t x)
        {
            if (this->begin_value())
                return this->key(tojsonstr(x));

            sink_->pp(x);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::number_unsigned(std::uint64_t x)
        {
            if (this->begin_value())
                return this->key(tojsonstr(x));

            sink_->pp(x);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::number(double x)
        {
            /* formatting: ppsink's double printer, for now --
             * .xo-backlog/xo-printjson/issues/10 decides json's own
             */
            if (this->begin_value())
                return this->key(tojsonstr(x));

            sink_->pp(x);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::number(float x)
        {
            return this->number(static_cast<double>(x));
        }

        JsonPpSink &
        JsonPpSink::boolean(bool x)
        {
            std::string_view text = (x ? "true" : "false");

            if (this->begin_value())
                return this->key(text);

            sink_->put(text);

            return *this;
        }

        JsonPpSink &
        JsonPpSink::null()
        {
            if (this->begin_value())
                return this->key("null");

            sink_->put("null");

            return *this;
        }

        JsonPpSink &
        JsonPpSink::raw(std::string_view text)
        {
            this->begin_nonkey_value("raw()");

            sink_->put(text);

            return *this;
        }

        // ----- bookkeeping -----

        bool
        JsonPpSink::begin_value()
        {
            Frame & f = stack_.back();

            switch (f.kind_) {
            case frame_kind::document:
                /* top-level values: the caller separates them, if at all */
                return false;
            case frame_kind::list:
                if (!f.first_)
                    this->separator(f.sep_spaces_);
                f.first_ = false;
                return false;
            case frame_kind::object:
                if (f.want_value_) {
                    f.want_value_ = false;
                    return false;
                }
                /* a key is expected: the value becomes it */
                return true;
            }

            return false;
        }

        void
        JsonPpSink::begin_nonkey_value(char const * what)
        {
            if (this->begin_value())
                this->misuse(what);   /* in key position: json keys are strings */
        }

        void
        JsonPpSink::open_group(frame_kind kind, char bracket, std::uint32_t sep_spaces)
        {
            (*sink_)(json_open{bracket});

            stack_.push_back(Frame{kind, sep_spaces, true, false});
        }

        void
        JsonPpSink::close_group(frame_kind kind, char bracket)
        {
            Frame & f = stack_.back();

            if (f.kind_ != kind) {
                this->misuse(f.kind_ == frame_kind::document
                             ? "close with nothing open"
                             : "close does not match the open object or list");
            }
            if (f.want_value_)
                this->misuse("close_object() after a key with no value");

            stack_.pop_back();

            (*sink_)(json_close{bracket});
        }

        void
        JsonPpSink::separator(std::uint32_t spaces)
        {
            (*sink_)(json_sep{spaces});
        }

        void
        JsonPpSink::misuse(char const * what) const
        {
            throw std::logic_error(tostr("JsonPpSink: ", what));
        }
    } /*namespace json*/
} /*namespace xo*/

/* end JsonPpSink.cpp */
