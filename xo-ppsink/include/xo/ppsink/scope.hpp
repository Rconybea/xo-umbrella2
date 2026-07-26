/** @file scope.hpp
 *
 *  @author Roland Conybeare, Jul 2026
 *
 *  Minimal indenting scope-logger POC, built on xo::print::PpSink.
 *
 *  This is the arena-free logging front-end: it composes output through a
 *  PpSink (see xo::print::LogState for the per-thread active sink), so the
 *  same scope code drives flat output (FlatSink, here) and, later,
 *  arena-backed pretty output (PrettySink, in xo-indentlog2).
 *
 *  POC scope: entry/exit banners + per-nesting-level indentation + log().
 *  Deliberately omits (for now) color, timestamps, log levels, code location,
 *  and the function-name styling of the legacy xo-indentlog scope.
 *
 *  NOTE: this header is intentionally macro-free.  The terse XO_SCOPE/
 *  XO_ENTER0 convenience macros live in the opt-in header scope_macros.hpp
 *  so that scope.hpp can coexist in one translation unit with the legacy
 *  <xo/indentlog/scope.hpp> (which defines its own XO_* macros).
 **/

#pragma once

#include "PpSink.hpp"
#include "Pretty.hpp"
#include "LogState.hpp"
#include <string_view>
#include <ostream>   /* scope streams values via pp_write -> ins.os() << x */
#include <utility>
#include <cstdint>

namespace xo::print {
    /** @brief process-wide logging configuration (POC subset) **/
    struct scope_config {
        /** spaces of indentation per nesting level **/
        static inline std::uint32_t indent_width = 2;
    };

    /** @brief captured scope-entry information (POC subset)
     *
     *  Produced by the XO_ENTER0 macro (see scope_macros.hpp) and consumed by
     *  the scope constructor.  For now it only carries the scope name; log
     *  level, source location and function-name styling are deferred to the
     *  feature-parity pass.
     **/
    struct scope_setup {
        /** scope name (POC: __func__; later __PRETTY_FUNCTION__ + styling) **/
        std::string_view name_;
    };

    /** @brief RAII scope logger: logs entry on construction, exit on destruction,
     *  and log() lines in between, each at the current nesting indentation.
     **/
    class scope {
    public:
        /** enter a scope named @p name (no entry-banner arguments) **/
        explicit scope(std::string_view name)
            : name_{name}
        {
            begin_scope();
        }

        /** enter the scope described by @p setup, logging @p args on the entry
         *  banner (e.g. XO_ENTER0(always), ":x ", x)
         **/
        template <typename... Ts>
        explicit scope(scope_setup setup, Ts &&... args)
            : name_{setup.name_}
        {
            begin_scope(std::forward<Ts>(args)...);
        }

        ~scope();

        scope(const scope &) = delete;
        scope & operator=(const scope &) = delete;

        /** log one line: current indent, then each argument via pp_write **/
        template <typename... Ts>
        void log(Ts &&... args) {
            xo::print::LogState & st = xo::print::ThreadLogState::thread_log_state();
            xo::print::PpSink & sink = st.sink();

            emit_indent(st);
            sink.begin();
            (xo::print::pp_write(sink, args), ...);
            sink.end();
            sink.put("\n");
        }

    private:
        /** log the entry banner ("+name" + optional args) and increase nesting **/
        template <typename... Ts>
        void begin_scope(Ts &&... args) {
            xo::print::LogState & st = xo::print::ThreadLogState::thread_log_state();
            xo::print::PpSink & sink = st.sink();

            emit_indent(st);
            sink.put("+");
            sink.put(name_);
            if constexpr (sizeof...(args) > 0) {
                sink.put(" ");
                sink.begin();
                (xo::print::pp_write(sink, args), ...);
                sink.end();
            }
            sink.put("\n");

            st.incr_nesting();
        }

        /** write (nesting_level * indent_width) spaces to the active sink **/
        static void emit_indent(xo::print::LogState & st);

    private:
        /** scope name (e.g. function name); printed in the +/- entry/exit banners **/
        std::string_view name_;
    };
} /*namespace xo::print*/

/* end scope.hpp */
