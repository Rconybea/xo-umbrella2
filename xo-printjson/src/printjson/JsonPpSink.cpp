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
#include <cmath>
#include <stdexcept>
#include <string>

namespace xo {
    using xo::pp::escapestyle;
    using xo::pp::quot;
    using xo::pp::tojsonstr;
    using xo::pp::tostr;

    namespace json {
        namespace {
            /** room for any float or double's shortest text: at most 24 chars
             *  ("-2.2250738585072014e-308")
             **/
            constexpr std::size_t c_float_buf_z = 32;

            /** @p x as json text, in @p buf.
             *
             *  Finite: the SHORTEST text that reads back as the same @p x
             *  (std::to_chars with no format or precision) -- so 0.1 stays
             *  "0.1" and 1.23456789 is no longer cut to "1.23457".
             *  Locale-free.  See .xo-backlog/xo-printjson/issues/10
             *
             *  Non-finite: the json extension's bare tokens, NaN / Infinity /
             *  -Infinity -- JSON5, a javascript literal and python's
             *  json.loads take them; JSON.parse does not.  A setting choosing
             *  the rendering is to come (issues/10)
             **/
            template <typename T>
            std::string_view
            float_text(T x, char (&buf)[c_float_buf_z])
            {
                if (std::isnan(x))
                    return "NaN";
                if (std::isinf(x))
                    return (x > 0 ? "Infinity" : "-Infinity");

                auto [ptr, ec] = std::to_chars(buf, buf + c_float_buf_z, x);
                (void)ec;   /* cannot fail: buf holds any float's shortest text */

                return std::string_view(buf, static_cast<std::size_t>(ptr - buf));
            }
        } /*namespace*/

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
            this->open_group(frame_kind::object, '{');

            return *this;
        }

        JsonPpSink &
        JsonPpSink::close_object()
        {
            this->close_group(frame_kind::object, '}');

            return *this;
        }

        JsonPpSink &
        JsonPpSink::open_list()
        {
            this->begin_nonkey_value("open_list()");
            this->open_group(frame_kind::list, '[');

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
                this->separator();
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
            char buf[c_float_buf_z];

            return this->number_text(float_text(x, buf));
        }

        JsonPpSink &
        JsonPpSink::number(float x)
        {
            /* as a float: widened to double first, 0.1f would print as
             * 0.10000000149011612
             */
            char buf[c_float_buf_z];

            return this->number_text(float_text(x, buf));
        }

        JsonPpSink &
        JsonPpSink::number_text(std::string_view text)
        {
            if (this->begin_value())
                return this->key(text);

            sink_->put(text);

            return *this;
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
                    this->separator();
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
        JsonPpSink::open_group(frame_kind kind, char bracket)
        {
            (*sink_)(json_open{bracket});

            stack_.push_back(Frame{kind, true, false});
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
        JsonPpSink::separator()
        {
            (*sink_)(json_sep());
        }

        void
        JsonPpSink::misuse(char const * what) const
        {
            throw std::logic_error(tostr("JsonPpSink: ", what));
        }
    } /*namespace json*/
} /*namespace xo*/

/* end JsonPpSink.cpp */
