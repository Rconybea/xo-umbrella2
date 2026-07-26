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
 **/

#pragma once

#include "PpSink.hpp"
#include "Pretty.hpp"
#include "LogState.hpp"
#include <string_view>
#include <cstdint>

namespace xo {
    /** @brief process-wide logging configuration (POC subset) **/
    struct scope_config {
        /** spaces of indentation per nesting level **/
        static inline std::uint32_t indent_width = 2;
    };

    /** @brief RAII scope logger: logs entry on construction, exit on destruction,
     *  and log() lines in between, each at the current nesting indentation.
     **/
    class scope {
    public:
        explicit scope(std::string_view name);
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
        /** write (nesting_level * indent_width) spaces to the active sink **/
        static void emit_indent(xo::print::LogState & st);

    private:
        /** scope name (e.g. function name); printed in the +/- entry/exit banners **/
        std::string_view name_;
    };
} /*namespace xo*/

/* end scope.hpp */
