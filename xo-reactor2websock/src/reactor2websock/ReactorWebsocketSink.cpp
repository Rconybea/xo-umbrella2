/** @file ReactorWebsocketSink.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "ReactorWebsocketSink.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::reflect::Reflect;

    namespace web {
        ReactorWebsocketSink::ReactorWebsocketSink(rp<WebsocketSink> ws_sink)
            : ws_sink_{std::move(ws_sink)}
        {}

        rp<ReactorWebsocketSink>
        ReactorWebsocketSink::make(rp<WebsocketSink> const & ws_sink)
        {
            return new ReactorWebsocketSink(ws_sink);
        }

        auto
        ReactorWebsocketSink::sink_ev_type() const -> TypeDescr
        {
            /* no single type: polymorphic, see allow_polymorphic_source() */
            return Reflect::require<void>();
        }

        uint32_t
        ReactorWebsocketSink::n_in_ev() const
        {
            /* counted where the event is actually sent, so the two cannot
             * disagree
             */
            return ws_sink_->n_in_ev();
        }

        void
        ReactorWebsocketSink::attach_source(rp<AbstractSource> const & src)
        {
            src->attach_sink(this);
        }

        void
        ReactorWebsocketSink::notify_ev_tp(TaggedPtr const & ev_tp)
        {
            ws_sink_->notify_ev_tp(ev_tp);
        }

        void
        ReactorWebsocketSink::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            sink.pretty_struct("ReactorWebsocketSink",
                               field("name", name_),
                               field("stream", ws_sink_->stream_name()),
                               field("n_in_ev", this->n_in_ev()));
        }
    } /*namespace web*/
} /*namespace xo*/

/* end ReactorWebsocketSink.cpp */
