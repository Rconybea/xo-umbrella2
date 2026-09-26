/** @file ReactorWebsocketSink.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/reactor/AbstractSink.hpp>
#include <xo/websock/WebsocketSink.hpp>
#include <string>

namespace xo {
    namespace web {
        /** @brief lets a reactor source deliver into a websocket subscription.
         *
         *  A reactor::AbstractSink in front of a web::WebsocketSink.  Accepts
         *  events of any reflected type from any source -- polymorphic and
         *  volatile both -- and forwards each one, as a TaggedPtr, to the
         *  websocket sink, which renders it as json and sends it.
         *
         *  This is the reactor half of what used to be xo-websock's
         *  WebsocketSinkImpl.  It was split off on 2026-09-26 so that
         *  xo-websock need not depend on xo-reactor: the websocket side only
         *  ever needed dynamic dispatch from a TaggedPtr, and the typed-sink
         *  formalism belongs to the reactor.  See
         *  .xo-backlog/xo-websock/issues/02.
         *
         *  Normally created by the subscribe function that
         *  web::stream_endpoint_descr() builds, not directly.
         **/
        class ReactorWebsocketSink : public reactor::AbstractSink {
        public:
            using AbstractEventProcessor = reactor::AbstractEventProcessor;
            using AbstractSource = reactor::AbstractSource;

        public:
            /** adapter delivering into @p ws_sink **/
            static rp<ReactorWebsocketSink> make(rp<WebsocketSink> const & ws_sink);

            /** the websocket sink events are forwarded to **/
            rp<WebsocketSink> const & ws_sink() const { return ws_sink_; }

            // from AbstractEventProcessor

            virtual std::string const & name() const override { return name_; }
            virtual void set_name(std::string const & x) override { name_ = x; }
            /* 0 consumers: this is where events leave the reactor network */
            virtual void visit_direct_consumers(std::function<void (bp<AbstractEventProcessor>)> const &) override {}
            virtual void pretty(xo::pp::PpSink & sink) const override;

            // from AbstractSink

            /* the websocket side renders any reflected type as json */
            virtual bool allow_polymorphic_source() const override { return true; }
            virtual TypeDescr sink_ev_type() const override;
            /* each event is rendered before notify_ev_tp returns, so it need
             * not outlive the call
             */
            virtual bool allow_volatile_source() const override { return true; }
            virtual uint32_t n_in_ev() const override;
            virtual void attach_source(rp<AbstractSource> const & src) override;
            virtual void notify_ev_tp(TaggedPtr const & ev_tp) override;

        private:
            explicit ReactorWebsocketSink(rp<WebsocketSink> ws_sink);

        private:
            /* user-controlled name, see AbstractEventProcessor::name */
            std::string name_;
            /* events delivered here are forwarded to this sink */
            rp<WebsocketSink> ws_sink_;
        }; /*ReactorWebsocketSink*/
    } /*namespace web*/
} /*namespace xo*/

/* end ReactorWebsocketSink.hpp */
