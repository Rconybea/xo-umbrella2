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
        namespace {
            /* sends to one session of a webserver.
             *
             * Interim: one per SINK, as the std::function it replaced was, and
             * never closed.  Issue 05 replaces it with one sender per session,
             * owned and closed by the webserver.
             */
            class WebserverSessionSender : public WsSender {
            public:
                WebserverSessionSender(rp<Webserver> websrv, uint32_t session_id)
                    : websrv_{std::move(websrv)}, session_id_{session_id} {}

                void send_text(std::string text) override {
                    websrv_->send_text(session_id_, std::move(text));
                }

                bool is_open() const override { return true; }

            private:
                rp<Webserver> websrv_;
                uint32_t session_id_ = 0;
            };
        }

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
            WebsocketSinkImpl(rp<WsSender> sender,
                              rp<PrintJson> const & pjson,
                              std::string stream_name,
                              uint32_t sub_id)
                : sender_{std::move(sender)},
                  pjson_{std::move(pjson)},
                  stream_name_{std::move(stream_name)},
                  sub_id_{sub_id}
                {}

            virtual std::string const & stream_name() const override { return stream_name_; }
            virtual uint32_t n_in_ev() const override { return n_in_ev_; }
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override;
            virtual void pretty(xo::pp::PpSink & sink) const override;
            virtual std::string display_string() const override;

        private:
            /* delivers each finished message.
             * For a webserver-created sink this sends to a specific
             * websocket session.
             */
            rp<WsSender> sender_;
            /* print arbitrary reflected stuff as json */
            rp<PrintJson> pjson_;
            /* name for stream.
             * this will be the vale of the "stream" tag in
             * initiating subscription message
             *   {"cmd": "subscribe", "stream", "/this/stream/name"}
             * e.g. in python, for a reactor source:
             *   web.register_stream_endpoint(
             *       xo.reactor2websock.stream_endpoint_descr(kf, "/this/stream/name"))
             */
            std::string stream_name_;
            /* this subscription's server-assigned id, on every envelope */
            uint32_t sub_id_ = 0;
            /* count #of events received.  Also the next message's seq */
            uint32_t n_in_ev_ = 0;
        }; /*WebsocketSinkImpl*/

        void
        WebsocketSinkImpl::notify_ev_tp(TaggedPtr const & ev_tp)
        {
            scope log(XO_DEBUG_(true /*debug_flag*/));

            std::stringstream ss;

            /* format message envelope.  seq is 0-based: this message's seq
             * is the count of messages sent before it
             */
            ss << "{" << quot("stream") << ": " << quot(this->stream_name_)
               << ", " << quot("sub_id") << ": " << this->sub_id_
               << ", " << quot("seq") << ": " << this->n_in_ev_
               << ", " << quot("event") << ": ";

            /* format event as json */
            this->pjson_->print_tp(ev_tp, &ss);

            ss << "}";

            log && log("sending", xtag("msg", ss.str()));

            ++(this->n_in_ev_);

            /* send event via associated websocket */
            this->sender_->send_text(ss.str());

        } /*notify_ev_tp*/

        void
        WebsocketSinkImpl::pretty(xo::pp::PpSink & sink) const
        {
            using xo::pp::field;

            const void * addr = this;

            sink.pretty_struct("WebsocketSinkImpl",
                               field("addr", addr),
                               field("sub_id", sub_id_),
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
                            std::string const & stream_name,
                            uint32_t sub_id)
        {
            /* events arriving at this sink are sent only to session_id */
            return make(new WebserverSessionSender(websrv, session_id),
                        pjson,
                        stream_name,
                        sub_id);
        } /*make*/

        rp<WebsocketSink>
        WebsocketSink::make(rp<WsSender> sender,
                            rp<PrintJson> const & pjson,
                            std::string const & stream_name,
                            uint32_t sub_id)
        {
            return new WebsocketSinkImpl(std::move(sender), pjson, stream_name, sub_id);
        } /*make*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.cpp */
