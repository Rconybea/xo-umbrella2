/** @file scope.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Minimal indenting scope-logger POC, built on xo::pp::PpSink.
 *
 *  This is the arena-free logging front-end: it composes output through a
 *  PpSink (see xo::pp::LogState for the per-thread active sink), so the
 *  same scope code drives flat output (FlatSink, here) and, later,
 *  arena-backed pretty output (PrettySink, in xo-indentlog2).
 *
 *  POC scope: entry/exit banners + per-nesting-level indentation + log() +
 *  log-level gating.  Deliberately omits (for now) color, timestamps, code
 *  location, and the function-name styling of the legacy xo-indentlog scope.
 *
 *  NOTE: this header is intentionally macro-free.  The terse XO_SCOPE_/
 *  XO_ENTER0_ convenience macros live in the opt-in header scope_macros.hpp
 *  so that scope.hpp can coexist in one translation unit with the legacy
 *  <xo/indentlog/scope.hpp> (which defines its own XO_* macros).
 **/

#pragma once

#include "PpSink.hpp"
#include "pretty_ostream.hpp"   /* pretty(): scope logs arbitrary types, so it needs the operator<< fallback */
#include "LogState.hpp"
#include "log_level.hpp"
#include "color.hpp"
#include "function_name.hpp"
#include <string_view>
#include <utility>
#include <cstdint>

namespace xo::pp {
    /** @brief process-wide logging configuration (POC subset) **/
    struct scope_config {
        /** spaces of indentation per nesting level **/
        static inline std::uint32_t indent_width = 2;
        /** cap on total indentation (columns): indent stops growing past this,
         *  so deeply-nested scopes don't march off the right side
         **/
        static inline std::uint32_t max_indent_width = 32;
        /** a scope logs iff its level is at least this severe **/
        static inline log_level min_log_level = log_level::default_level;
        /** if true, prefix each line with a UTC time-of-day timestamp
         *  (banner lines show the time; mid-scope log() lines show a
         *  same-width blank pad so they align under the banner)
         **/
        static inline bool time_enabled = false;
        /** microsecond precision "HH:MM:SS.uuuuuu" vs millisecond "HH:MM:SS.mmm" **/
        static inline bool time_usec_flag = false;
        /** if true, show the nesting depth "(N)" after the +/- banner marker **/
        static inline bool nesting_level_enabled = false;
        /** color for the "(N)" nesting-depth display (when color enabled);
         *  xterm 153 = light blue (legacy used a near-white xterm 195)
         **/
        static inline color_spec_type nesting_level_color = color_spec_type::xterm(153);
        /** color for the "+name" entry banner (default none => uncolored) **/
        static inline color_spec_type function_entry_color = color_spec_type::none();
        /** color for the "-name" exit banner (default none => uncolored) **/
        static inline color_spec_type function_exit_color = color_spec_type::none();
        /** how to style a __PRETTY_FUNCTION__ banner name (see XO_ENTER0_);
         *  streamlined => "Class::method"
         **/
        static inline xo::FunctionStyle function_style = xo::FunctionStyle::streamlined;
    };

    /** @brief captured scope-entry information (POC subset)
     *
     *  Produced by the XO_ENTER0_ macro (see scope_macros.hpp) and consumed by
     *  the scope constructor.  For now it carries the scope name and level;
     *  source location and function-name styling are deferred to the
     *  feature-parity pass.
     **/
    struct scope_setup {
        /** scope name -- typically __PRETTY_FUNCTION__ (see XO_ENTER0_), styled
         *  per @ref style_ when the banner is printed
         **/
        std::string_view name_;
        /** severity of this scope; gated against scope_config::min_log_level **/
        log_level level_ = log_level::always;
        /** how to style @ref name_ in the banner (default literal => verbatim) **/
        xo::FunctionStyle style_ = xo::FunctionStyle::literal;

        /** true iff a scope entered with this setup should log **/
        bool is_enabled() const { return level_ >= scope_config::min_log_level; }
    };

    /** @brief RAII scope logger: logs entry on construction, exit on destruction,
     *  and log() lines in between, each at the current nesting indentation.
     *
     *  A scope whose level is below scope_config::min_log_level is *disabled*:
     *  it emits nothing and does not affect nesting.  The idiom
     *  @code log && log(...) @endcode uses operator bool() to skip the log()
     *  call (and its argument evaluation) for a disabled scope.
     **/
    class scope {
    public:
        /** enter a scope named @p name (always enabled; no entry-banner args) **/
        explicit scope(std::string_view name)
            : name_{name}, finalized_{false}
        {
            begin_scope();
        }

        /** enter the scope described by @p setup, logging @p args on the entry
         *  banner (e.g. XO_ENTER0_(info), ":n ", n)
         **/
        template <typename... Ts>
        explicit scope(scope_setup setup, Ts &&... args)
            : name_{setup.name_}, style_{setup.style_}, finalized_{!setup.is_enabled()}
        {
            begin_scope(std::forward<Ts>(args)...);
        }

        ~scope() {
            if (!finalized_)
                end_scope();
        }

        scope(const scope &) = delete;
        scope & operator=(const scope &) = delete;

        /** true while this scope is enabled (and not yet finalized) **/
        bool enabled() const { return !finalized_; }
        operator bool() const { return enabled(); }

        /** log one line: current indent, then each argument via pretty.
         *  No-op (returns false) if the scope is disabled.
         **/
        template <typename... Ts>
        bool log(Ts &&... args) {
            if (!enabled())
                return false;

            xo::pp::LogState & st = xo::pp::ThreadLogState::thread_log_state();
            xo::pp::PpSink & sink = st.sink();

            emit_time(sink, false /*real_time: log() lines get a blank time pad*/);
            emit_indent(st);
            sink.begin();
            (xo::pp::pretty(sink, args), ...);
            sink.end();
            sink.put("\n");

            return true;
        }

        /** log(args...) spelled as a call, for the @c log && log(...) idiom **/
        template <typename... Ts>
        bool operator()(Ts &&... args) {
            return log(std::forward<Ts>(args)...);
        }

        /** optionally end the scope early (before dtor), logging @p args on the
         *  exit banner (e.g. end_scope("<- :retval ", retval)).  Idempotent.
         **/
        template <typename... Ts>
        void end_scope(Ts &&... args) {
            if (finalized_)
                return;
            finalized_ = true;

            xo::pp::LogState & st = xo::pp::ThreadLogState::thread_log_state();
            xo::pp::PpSink & sink = st.sink();

            st.decr_nesting();

            emit_time(sink, true /*real_time*/);
            emit_indent(st);
            sink.put("-");
            emit_nesting_level(sink, st.nesting_level());
            {
                color_guard g(sink, scope_config::function_exit_color);
                put_function_name(sink, style_, name_);
            }
            if constexpr (sizeof...(args) > 0) {
                sink.put(" ");
                sink.begin();
                (xo::pp::pretty(sink, args), ...);
                sink.end();
            }
            sink.put("\n");
        }

    private:
        /** log the entry banner ("+name" + optional args) and increase nesting.
         *  No-op if the scope is disabled.
         **/
        template <typename... Ts>
        void begin_scope(Ts &&... args) {
            if (!enabled())
                return;

            xo::pp::LogState & st = xo::pp::ThreadLogState::thread_log_state();
            xo::pp::PpSink & sink = st.sink();

            emit_time(sink, true /*real_time*/);
            emit_indent(st);
            sink.put("+");
            emit_nesting_level(sink, st.nesting_level());
            {
                color_guard g(sink, scope_config::function_entry_color);
                put_function_name(sink, style_, name_);
            }
            if constexpr (sizeof...(args) > 0) {
                sink.put(" ");
                sink.begin();
                (xo::pp::pretty(sink, args), ...);
                sink.end();
            }
            sink.put("\n");

            st.incr_nesting();
        }

        /** write (nesting_level * indent_width, capped) spaces to the active sink **/
        static void emit_indent(xo::pp::LogState & st);

        /** if timestamps are enabled, write the leftmost time field to @p sink:
         *  the real UTC time when @p real_time, else a same-width blank pad
         *  (so mid-scope log() lines align under the timestamped banner)
         **/
        static void emit_time(xo::pp::PpSink & sink, bool real_time);

        /** if nesting_level_enabled, write "(N) " (N = @p level, colored) to
         *  @p sink -- the depth display shown after the +/- banner marker
         **/
        static void emit_nesting_level(xo::pp::PpSink & sink, std::uint32_t level);

    private:
        /** scope name (e.g. function name); printed in the +/- entry/exit banners **/
        std::string_view name_;
        /** how to style @ref name_ in the banner (literal for the string_view
         *  ctor; from scope_setup / scope_config::function_style via XO_ENTER0_)
         **/
        xo::FunctionStyle style_ = xo::FunctionStyle::literal;
        /** once true, logging is disabled; set at entry (level gating) or by
         *  end_scope(); guards against a double exit banner from the dtor
         **/
        bool finalized_ = false;
    };
} /*namespace xo::pp*/

/* end scope.hpp */
