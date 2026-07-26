/** @file scope.cpp **/

#include <xo/ppsink/scope.hpp>
#include <string>

namespace xo::print {
    void
    scope::emit_indent(xo::print::LogState & st) {
        std::string pad(st.nesting_level() * scope_config::indent_width, ' ');
        st.sink().put(pad);
    }

    scope::scope(std::string_view name)
        : name_{name}
    {
        xo::print::LogState & st = xo::print::ThreadLogState::thread_log_state();
        xo::print::PpSink & sink = st.sink();

        emit_indent(st);
        sink.put("+");
        sink.put(name_);
        sink.put("\n");

        st.incr_nesting();
    }

    scope::~scope() {
        xo::print::LogState & st = xo::print::ThreadLogState::thread_log_state();
        xo::print::PpSink & sink = st.sink();

        st.decr_nesting();

        emit_indent(st);
        sink.put("-");
        sink.put(name_);
        sink.put("\n");
    }
} /*namespace xo::print*/

/* end scope.cpp */
