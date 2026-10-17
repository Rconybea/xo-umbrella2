/** @file JsonPpSink.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include <xo/ppsink/PpSink.hpp>
#include <xo/ppsink/Prettifier.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

namespace xo {
    namespace json {
        /** @brief json pretty-printer, providing structural entry points.
         *
         *  A wrapper around a PpSink.  Where a printer once wrote text --
         *  put("["), quot(k), ": " -- it says what the text is:
         *
         *    json.open_object()
         *        .key("size").number(3)
         *        .key("tags").open_list().string("a").string("b").close_list()
         *        .close_object();
         *
         *  The wrapper inserts separators, checks that keys and values
         *  alternate, and owns the json policy those calls need: escaping
         *  (the sink must escape for json), number formatting, and layout
         *  (each object and list is a PpSink group: one line if it fits,
         *  one item per line if not).  See .xo-backlog/xo-printjson/issues/11
         *
         *  Misuse -- a key in a list, a value where a list or object cannot
         *  go, a closing bracket that does not match -- throws
         *  std::logic_error: a printer bug, not a data condition.
         **/
        class JsonPpSink {
        public:
            using PpSink = xo::pp::PpSink;

        public:
            /** json into @p sink.  Throws std::runtime_error unless @p sink
             *  escapes for json (@c sink->style().escape == escapestyle::json,
             *  e.g. PpStyle::json()): a display-escaping sink would write
             *  \\xNN into the document.  Colour is not checked: coloured json,
             *  for a human to read, is the caller's choice
             **/
            explicit JsonPpSink(PpSink * sink);

            JsonPpSink(JsonPpSink const &) = delete;
            JsonPpSink & operator=(JsonPpSink const &) = delete;

            /** begin an object: keys and values follow, alternating **/
            JsonPpSink & open_object();
            /** end the object open_object() began **/
            JsonPpSink & close_object();

            /** begin a list.  @p sep_spaces: spaces after each "," when the
             *  list fits on a line -- 1 gives "[1, 2]", 0 gives "[1,2]"
             **/
            JsonPpSink & open_list(std::uint32_t sep_spaces = 1);
            /** end the list open_list() began **/
            JsonPpSink & close_list();

            /** the next key of the open object **/
            JsonPpSink & key(std::string_view k);

            /** @defgroup json-values values
             *
             *  In a list, or after a key: a value.  In an object where a key
             *  is expected: that key -- json keys are strings, so a number,
             *  boolean or null renders as the key spelled the same way.
             **/
            ///@{

            JsonPpSink & string(std::string_view s);

            /** an integer: not bool, not a character type -- the same
             *  set ppsink prints as a number (xo::pp::pp_number_integral)
             **/
            template <typename T>
                requires xo::pp::pp_number_integral<T>
            JsonPpSink & number(T x) {
                if constexpr (std::is_signed_v<T>)
                    return this->number_signed(static_cast<std::int64_t>(x));
                else
                    return this->number_unsigned(static_cast<std::uint64_t>(x));
            }

            JsonPpSink & number(double x);
            JsonPpSink & number(float x);

            JsonPpSink & boolean(bool x);
            JsonPpSink & null();

            /** a value written verbatim: for text that is not json (a
             *  diagnostic) or a json extension (NaN).  A last resort -- each
             *  use is listed in .xo-backlog/xo-printjson/issues/11.  Not
             *  allowed where a key is expected
             **/
            JsonPpSink & raw(std::string_view text);

            ///@}

        private:
            enum class frame_kind : std::uint8_t { document, object, list };

            struct Frame {
                frame_kind kind_ = frame_kind::document;
                /** list: spaces after a separator, when the list fits **/
                std::uint32_t sep_spaces_ = 1;
                /** no item written yet: no separator before the next **/
                bool first_ = true;
                /** object: a key has been written, its value has not **/
                bool want_value_ = false;
            };

            /** account for a value about to be written: separator (list),
             *  or consume the key (object).  @return true iff the value is in
             *  KEY position -- the open object expects a key, not a value
             **/
            bool begin_value();
            /** begin_value(), for a value that cannot be a key: throws **/
            void begin_nonkey_value(char const * what);

            /** an object or list opens: its bracket, and the layout group **/
            void open_group(frame_kind kind, char bracket, std::uint32_t sep_spaces);
            /** the group open_group() began closes **/
            void close_group(frame_kind kind, char bracket);

            /** "," and a place to break **/
            void separator(std::uint32_t spaces);

            JsonPpSink & number_signed(std::int64_t x);
            JsonPpSink & number_unsigned(std::uint64_t x);

            [[noreturn]] void misuse(char const * what) const;

        private:
            /** where json goes **/
            PpSink * sink_ = nullptr;
            /** open groups; the bottom one is the document **/
            std::vector<Frame> stack_;
        };
    } /*namespace json*/
} /*namespace xo*/

/* end JsonPpSink.hpp */
