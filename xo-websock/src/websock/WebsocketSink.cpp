/* file WebsocketSink.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "WebsocketSink.hpp"
#include "Webserver.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/indentlog2/print/tostr.hpp>  /* display_string */
#include <xo/ppsink/quoted_ostream.hpp>   /* ss << quot(..) */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* ss << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::TaggedPtr;
    using xo::pp::quot;
    using xo::pp::scope;
    using xo::pp::xtag;

    namespace web {
        /* a sink that publishes to a websocket.
         * The websocket api creates a WebsocketSink instance
         * on behalf of an incoming subscription request.
         * application code will hold onto the sink somewhere
         * and publish events to it,  to send them via websocket.
         */
        class WebsocketSinkImpl : public WebsocketSink {
        public:
            using PrintJson = xo::json::PrintJson;

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

            virtual std::string const & stream_name() const override { return stream_name_; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override;
            virtual void pretty(xo::pp::PpSink & sink) const override;
            virtual std::string display_string() const override;

        private:
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
             * e.g. in python, for a reactor source:
             *   web.register_stream_endpoint(
             *       xo.reactor2websock.stream_endpoint_descr(kf, "/this/stream/name"))
             */
            std::string stream_name_;
            /* count #of events received */
            uint32_t n_in_ev_ = 0;
        }; /*WebsocketSinkImpl*/

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
                               field("n_in_ev", n_in_ev_),
                               field("stream", stream_name_));
        }

        std::string
        WebsocketSinkImpl::display_string() const
        {
            using xo::pp::tostr;

            /* same shape as Webserver::display_string */
            WebsocketSinkImpl * self = const_cast<WebsocketSinkImpl *>(this);

            return tostr(rp<WebsocketSinkImpl>(self));
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
