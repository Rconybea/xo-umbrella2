/* file WebsocketSink.cpp
 *
 * author: Roland Conybeare, Sep 2022
 */

#include "WebsocketSink.hpp"
#include "webserver_json.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/printjson/JsonMembers.hpp>
#include <xo/printjson/JsonObject.hpp>
#include <xo/printjson/type_keys.hpp>
#include <xo/reflect/TaggedPtr.hpp>
#include <xo/indentlog2/print/tostr.hpp>  /* display_string */
#include <xo/reflect/Reflect.hpp>
#include <xo/ppsink/quoted_ostream.hpp>   /* ss << quot(..) */
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/tag_ostream.hpp>      /* ss << xtag(..) */
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */
#include <xo/reflect/StructReflector.hpp>

namespace xo {
    using xo::json::PrintJson;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;
    using xo::reflect::TaggedRcptr;
    using xo::reflect::Reflect;
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
            using TaggedRcptr = xo::reflect::TaggedRcptr;

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
            virtual void print_json(json::JsonPrintState & state) const override;
            virtual void pretty(xo::pp::PpSink & sink) const override;
            virtual std::string display_string() const override;
            virtual TaggedRcptr self_tp() override;

        private:
            friend class WebsocketSink;

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

        void
        WebsocketSinkImpl::print_json(json::JsonPrintState & state) const
        {
            /* the sender: held, not owned -- printed in full under its
             * session, as its actual type, so a ref by most-derived address
             */
            void const * sender = dynamic_cast<void const *>(this->sender_.get());

            json::JsonObject obj = state.open_object("WebsocketSink",
                                                     Reflect::require<WebsocketSinkImpl>());

            /* refcount: the router's subscription slot, plus whatever the
             * application holds (e.g. a source it is attached to).
             * seq: read without a lock -- a source may be sending now
             */
            obj.key("refcount", this->reference_counter())
                .key("stream", this->stream_name_)
                .key("sub_id", this->sub_id_)
                .key("seq", this->n_in_ev_)
                .key_ref("sender", sender);

            /* chosen C++ members (.xo-backlog/xo-websock/issues/13) */
            obj.members()
                .member_ref<rp<WsSender>>("sender_", sender)
                /* the server's, shared: printed in full in the server's members */
                .member_ref<rp<PrintJson>>("pjson_", this->pjson_.get())
                .member("stream_name_", this->stream_name_)
                .member("sub_id_", this->sub_id_)
                /* read without a lock: a source may be sending now */
                .member("n_in_ev_", this->n_in_ev_)
                .end();

            obj.close();
        } /*print_json*/

        TaggedRcptr
        WebsocketSinkImpl::self_tp()
        {
            return Reflect::make_rctp(this);
        }

        // ----- WebsocketSink -----

        void
        WebsocketSink::print_json(json::JsonPrintState & state) const
        {
            json::JsonObject obj = state.open_object("WebsocketSink",
                                                     Reflect::require<WebsocketSink>());

            obj.key("refcount", this->reference_counter())
                .key("stream", this->stream_name());

            obj.close();
        } /*print_json*/

        rp<WebsocketSink>
        WebsocketSink::make(rp<WsSender> sender,
                            rp<PrintJson> const & pjson,
                            std::string const & stream_name,
                            uint32_t sub_id)
        {
            return new WebsocketSinkImpl(std::move(sender), pjson, stream_name, sub_id);
        } /*make*/

        void
        WebsocketSink::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            /* no members yet: a member is added as a printer opts in to
             * show it (.xo-backlog/xo-websock/issues/13)
             */
            {
                StructReflector<WebsocketSink> sr;

                if (sr.is_incomplete()) {
                    //sr.adopt_ancestors<SelfTaggingDisplayable>();  // need SelfTaggingDisplayable reflected
                }
            }

            {
                StructReflector<WebsocketSinkImpl> sr;

                if (sr.is_incomplete()) {
                    sr.adopt_ancestors<WebsocketSink>();

                    REFLECT_MEMBER(sr, sender);
                    REFLECT_MEMBER(sr, pjson);
                    REFLECT_MEMBER(sr, stream_name);
                    REFLECT_MEMBER(sr, sub_id);
                    REFLECT_MEMBER(sr, n_in_ev);
                }
            }
        } /*reflect_self*/

    } /*namespace web*/
} /*namespace xo*/

/* end WebsocketSink.cpp */
