/* file WebsocketSink.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "WebsocketSink.hpp"
#include "Webserver.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* ss << quot(..) */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* ss << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::reactor::AbstractSource;
    using xo::json::PrintJson;
    using xo::reflect::Reflect;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TypeDescr;
    /* legacy quot(x) was just quot(x) for a char const *;
     * xo::pp::quot takes std::string_view, so it covers both.
     */

    namespace web {
        /* NB these sit in namespace xo::web, NOT namespace xo.
         * xo-reactor's public headers (AbstractSink.hpp, Sink.hpp) still
         * include legacy xo-indentlog, so xo::xtag is visible in this TU.
         * A using-declaration at namespace-xo scope would be *ambiguous*
         * with it rather than shadowing it; one scope further in, ordinary
         * unqualified lookup stops here and never reaches xo::xtag.
         */
        using xo::pp::quot;
        using xo::pp::scope;
        using xo::pp::xtag;

        /* a sink that publishes to a websocket.
         * The websocket api creates a WebsocketSink instance
         * on behalf of an incoming subscription request.
         * application code will hold onto the sink somewhere
         * and publish events to it,  to send them via websocket.
         */
        class WebsocketSinkImpl : public WebsocketSink {
        public:
            using PrintJson = xo::json::PrintJson;
            using AbstractSource = reactor::AbstractSource;

        public:
            WebsocketSinkImpl(rp<Webserver> const & websrv,
                              rp<PrintJson> const & pjson,
                              uint32_t session_id,
                              std::string stream_name)
                : websrv_{std::move(websrv)},
                  pjson_{std::move(pjson)},
                  session_id_{session_id},
                  stream_name_{std::move(stream_name)}
                {}

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { this->name_ = x; }
            /* 0 consumers for websocket sink,  since it's not a source */
            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor>)> const &) override {}
            virtual void pretty(xo::pp::PpSink & sink) const override;

            virtual bool allow_polymorphic_source() const override { return true; }
            virtual TypeDescr sink_ev_type() const override;
            virtual bool allow_volatile_source() const override { return true; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void attach_source(rp<AbstractSource> const & src) override;
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override;

        private:
            /* (ideally unique) user-controlled name for this sink
             * in practice not likely to be accessible,
             * so probably want to generate a unique-y default
             */
            std::string name_;
            /* webserver implementation */
            rp<Webserver> websrv_;
            /* print arbitrary reflected stuff as json */
            rp<PrintJson> pjson_;
            /* websocket session id# - events arriving at this sink
             * will be sent only to the session identified by .session_id
             */
            uint32_t session_id_;
            /* name for stream.
             * this will be the vale of the "stream" tag in
             * initiating subscription message
             *   {"cmd": "subscribe", "stream", "/this/stream/name"}
             * e.g. in python:
             *   web.register_stream_endpoint(kf.stream_endpoint_descr("/this/stream/name"))
             */
            std::string stream_name_;
            /* count #of events received */
            uint32_t n_in_ev_ = 0;
        }; /*WebsocketSinkImpl*/

        TypeDescr
        WebsocketSinkImpl::sink_ev_type() const
        {
            return Reflect::require<void>();
        } /*sink_ev_type*/

        void
        WebsocketSinkImpl::attach_source(rp<AbstractSource> const & src) {
            src->attach_sink(this);
        } /*attach_source*/

        void
        WebsocketSinkImpl::notify_ev_tp(TaggedPtr const & ev_tp)
        {
            scope log(XO_DEBUG_(true /*debug_flag*/));

            std::stringstream ss;

            /* format message envelope */
            ss << "{" << quot("stream") << ": " << quot(this->stream_name_)
               << ", " << quot("event") << ": ";

            /* format event as json */
            this->pjson_->print_tp(ev_tp, &ss);

            ss << "}";

            log && log("sending", xtag("msg", ss.str()));

            ++(this->n_in_ev_);

            /* send event via associated websocket */
            this->websrv_->send_text(this->session_id_, ss.str());

        } /*notify_ev_tp*/

        void
        WebsocketSinkImpl::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            const void * addr = this;

            sink.pretty_struct("WebsocketSinkImpl",
                               field("addr", addr),
                               field("name", name_),
                               field("n_in_ev", n_in_ev_),
                               field("stream", stream_name_));
        }

        // ----- WebsocketSink -----

        rp<WebsocketSink>
        WebsocketSink::make(rp<Webserver> const & websrv,
                            rp<PrintJson> const & pjson,
                            uint32_t session_id,
                            std::string const & stream_name)
        {
            return new WebsocketSinkImpl(websrv, pjson, session_id, stream_name);
        } /*make*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.cpp */
