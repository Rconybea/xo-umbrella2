/** @file WsSessionRouter.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  Inbound websocket commands and the outbound envelope --
 *  .xo-backlog/xo-websock/issues/04.
 *
 *  No socket anywhere.  WsSessionRouter reaches the server only through three
 *  injected functions (find an endpoint, make a sink, reply to the session),
 *  so each case wires those to recorders and drives perform_cmd() with the
 *  exact text a browser would send.  The sink cases use the REAL sink
 *  implementation through WebsocketSink::make(send_fn, ...), whose output is
 *  the envelope a browser would receive.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/WsSessionRouter.hpp"
#include "xo/websock/DynamicEndpoint.hpp"
#include "xo/websock/WebsocketSink.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <json/json.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace xo {
    using xo::web::WsSessionRouter;
    using xo::web::DynamicEndpoint;
    using xo::web::WebsocketSink;
    using xo::json::PrintJson;
    using xo::json::PrintJsonSingleton;
    using xo::reflect::Reflect;
    using xo::fn::CallbackId;

    namespace ut {
        namespace {
            /** parse @p text, which must be valid json **/
            Json::Value parse(std::string const & text) {
                Json::Value root;
                JSONCPP_STRING err;
                std::unique_ptr<Json::CharReader> rd(Json::CharReaderBuilder().newCharReader());

                bool ok = rd->parse(text.data(), text.data() + text.size(), &root, &err);

                INFO("text: " << text << " err: " << err);
                REQUIRE(ok);

                return root;
            }

            /** stands in for the webserver's per-subscription sink **/
            class FakeSink : public WebsocketSink {
            public:
                explicit FakeSink(std::string stream) : stream_{std::move(stream)} {}

                std::string const & stream_name() const override { return stream_; }
                uint32_t n_in_ev() const override { return n_in_ev_; }
                void notify_ev_tp(TaggedPtr const &) override { ++n_in_ev_; }
                void pretty(xo::pp::PpSink & sink) const override { sink.put("<FakeSink>"); }
                std::string display_string() const override { return "<FakeSink>"; }

                std::string stream_;
                uint32_t n_in_ev_ = 0;
            };

            /** everything a router did, as seen from outside it **/
            struct Recorder {
                /* sinks the endpoints' subscribe functions were handed */
                std::vector<rp<WebsocketSink>> subscribed_v_;
                /* (sink, msg) pairs the receive functions were handed */
                std::vector<std::pair<rp<WebsocketSink>, Json::Value>> received_v_;
                /* callback ids the unsubscribe functions were handed */
                std::vector<uint32_t> unsubscribed_v_;
                /* error replies, parsed */
                std::vector<Json::Value> reply_v_;
            };

            /** endpoints by stream name, plus a router over them **/
            struct Fixture {
                Recorder rec_;
                std::map<std::string, std::unique_ptr<DynamicEndpoint>> endpoint_map_;

                /* how the router makes a sink.  FakeSink unless a case needs
                 * the real envelope
                 */
                WsSessionRouter::SinkFactory sink_fn_
                    = [](std::string const & stream) -> rp<WebsocketSink> {
                          return new FakeSink(stream);
                      };

                /** an endpoint for @p stream; @p with_receive adds a receive fn **/
                void add_endpoint(std::string const & stream, bool with_receive = true) {
                    Recorder * rec = &rec_;

                    auto sub_fn = [rec](rp<WebsocketSink> const & sink) {
                        rec->subscribed_v_.push_back(sink);
                        return CallbackId(static_cast<uint32_t>(rec->subscribed_v_.size()));
                    };
                    auto unsub_fn = [rec](CallbackId id) {
                        rec->unsubscribed_v_.push_back(id.id());
                    };
                    xo::web::StreamReceiveFn recv_fn = nullptr;

                    if (with_receive) {
                        recv_fn = [rec](rp<WebsocketSink> const & sink, Json::Value const & msg) {
                            rec->received_v_.push_back({sink, msg});
                        };
                    }

                    endpoint_map_[stream]
                        = DynamicEndpoint::make_stream(stream, sub_fn, unsub_fn, recv_fn);
                }

                std::unique_ptr<WsSessionRouter> make_router() {
                    Recorder * rec = &rec_;
                    auto * epmap = &endpoint_map_;

                    return std::make_unique<WsSessionRouter>(
                        [epmap](std::string const & stream) -> DynamicEndpoint * {
                            auto ix = epmap->find(stream);
                            return (ix == epmap->end()) ? nullptr : ix->second.get();
                        },
                        sink_fn_,
                        [rec](std::string text) {
                            rec->reply_v_.push_back(parse(text));
                        });
                }
            };
        } /*namespace*/

        TEST_CASE("subscribe-hands-the-endpoint-a-sink-for-its-stream", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE(fx.rec_.subscribed_v_.size() == 1);
            REQUIRE(fx.rec_.subscribed_v_[0]->stream_name() == "/fw");
            REQUIRE(router->n_subscription() == 1);
            REQUIRE(fx.rec_.reply_v_.empty());
        }

        TEST_CASE("subscribe-to-an-unknown-stream-stays-silent", "[websock][router]")
        {
            /* unchanged behaviour, deliberately: only the send path gained
             * error replies (issue 04)
             */
            Fixture fx;
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/nope"})");

            REQUIRE(router->n_subscription() == 0);
            REQUIRE(fx.rec_.reply_v_.empty());
        }

        TEST_CASE("send-reaches-receive-with-the-subscriptions-own-sink", "[websock][router]")
        {
            /* the point of tying messages to subscriptions: the handler gets
             * the SAME sink the subscribe function got, so its reply goes back
             * to the session that asked, with no bookkeeping
             */
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "send", "stream": "/fw", "msg": {"op": "step", "n": 2}})");

            REQUIRE(fx.rec_.received_v_.size() == 1);
            REQUIRE(fx.rec_.received_v_[0].first.get() == fx.rec_.subscribed_v_[0].get());

            Json::Value const & msg = fx.rec_.received_v_[0].second;

            REQUIRE(msg["op"].asString() == "step");
            REQUIRE(msg["n"].asInt() == 2);
            REQUIRE(fx.rec_.reply_v_.empty());
        }

        TEST_CASE("send-routes-to-the-named-stream-only", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/b");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");
            router->perform_cmd(R"({"cmd": "send", "stream": "/b", "msg": "step"})");

            REQUIRE(fx.rec_.received_v_.size() == 1);
            REQUIRE(fx.rec_.received_v_[0].first->stream_name() == "/b");
            REQUIRE(fx.rec_.received_v_[0].second.asString() == "step");
        }

        TEST_CASE("send-without-a-subscription-is-an-error", "[websock][router]")
        {
            /* the endpoint exists and accepts messages; THIS session just
             * never subscribed to it
             */
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "send", "stream": "/fw", "msg": "step"})");

            REQUIRE(fx.rec_.received_v_.empty());
            REQUIRE(fx.rec_.reply_v_.size() == 1);
            REQUIRE(fx.rec_.reply_v_[0]["stream"].asString() == "/fw");
            REQUIRE(fx.rec_.reply_v_[0]["error"].asString() == "not subscribed to stream");
        }

        TEST_CASE("send-to-a-stream-without-a-receive-fn-is-an-error", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw", false /*with_receive*/);
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "send", "stream": "/fw", "msg": "step"})");

            REQUIRE(fx.rec_.reply_v_.size() == 1);
            REQUIRE(fx.rec_.reply_v_[0]["error"].asString() == "stream does not accept messages");
        }

        TEST_CASE("bad-messages-get-an-error-and-never-throw", "[websock][router]")
        {
            /* before the router, a json string or array crashed the server:
             * jsoncpp throws from operator[] on a non-object, and the
             * exception escaped into libwebsockets' C callback
             */
            Fixture fx;
            auto router = fx.make_router();

            REQUIRE_NOTHROW(router->perform_cmd("not json at all"));
            REQUIRE_NOTHROW(router->perform_cmd(R"("a bare string")"));
            REQUIRE_NOTHROW(router->perform_cmd(R"([1, 2, 3])"));
            REQUIRE_NOTHROW(router->perform_cmd(R"({"cmd": "send"})"));

            REQUIRE(fx.rec_.reply_v_.size() == 4);
            for (auto const & reply : fx.rec_.reply_v_) {
                INFO("reply error: " << reply["error"].asString());
                REQUIRE(reply.isMember("error"));
                /* none of these can name a stream */
                REQUIRE(!reply.isMember("stream"));
            }

            /* a non-string cmd is an unknown command: ignored, as before */
            REQUIRE_NOTHROW(router->perform_cmd(R"({"cmd": 3, "stream": "/fw"})"));
            REQUIRE(fx.rec_.reply_v_.size() == 4);
        }

        TEST_CASE("a-throwing-handler-becomes-an-error-reply", "[websock][router]")
        {
            Fixture fx;
            fx.endpoint_map_["/fw"]
                = DynamicEndpoint::make_stream(
                      "/fw",
                      [](rp<WebsocketSink> const &) { return CallbackId(1); },
                      [](CallbackId) {},
                      [](rp<WebsocketSink> const &, Json::Value const &) {
                          throw std::runtime_error("boom");
                      });
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE_NOTHROW(router->perform_cmd(R"({"cmd": "send", "stream": "/fw"})"));
            REQUIRE(fx.rec_.reply_v_.size() == 1);
            REQUIRE(fx.rec_.reply_v_[0]["error"].asString() == "stream handler failed: boom");
        }

        TEST_CASE("unsubscribe-all-releases-every-subscription", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/b");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");

            router->unsubscribe_all();

            /* each with the id its subscribe function returned */
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{1, 2});
            REQUIRE(router->n_subscription() == 0);
        }

        TEST_CASE("seq-is-zero-based-and-per-subscription", "[websock][sink][seq]")
        {
            /* the real sink, handing its finished messages to a recorder: the
             * text here is exactly what a browser would receive
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();

            std::vector<std::string> out_a;
            std::vector<std::string> out_b;

            rp<WebsocketSink> a = WebsocketSink::make(
                [&out_a](std::string t) { out_a.push_back(std::move(t)); }, pjson, "/a");
            rp<WebsocketSink> b = WebsocketSink::make(
                [&out_b](std::string t) { out_b.push_back(std::move(t)); }, pjson, "/b");

            int ev = 42;

            a->notify_ev_tp(Reflect::make_tp(&ev));
            a->notify_ev_tp(Reflect::make_tp(&ev));
            b->notify_ev_tp(Reflect::make_tp(&ev));
            a->notify_ev_tp(Reflect::make_tp(&ev));

            REQUIRE(out_a.size() == 3);
            REQUIRE(out_b.size() == 1);

            for (int i = 0; i < 3; ++i) {
                Json::Value env = parse(out_a[i]);

                INFO("envelope: " << out_a[i]);
                REQUIRE(env["stream"].asString() == "/a");
                REQUIRE(env["seq"].asInt() == i);
                REQUIRE(env["event"].asInt() == 42);
            }

            /* b's count is its own: interleaving with a does not advance it */
            REQUIRE(parse(out_b[0])["seq"].asInt() == 0);
            REQUIRE(a->n_in_ev() == 3);
        }

        TEST_CASE("a-handlers-reply-goes-out-through-its-subscription", "[websock][router][sink]")
        {
            /* end to end short of a socket: real sinks from the router's sink
             * factory, a handler that replies through the sink it was handed,
             * and the reply arriving -- enveloped and sequenced -- on that
             * subscription's outbox only
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();
            std::map<std::string, std::vector<std::string>> outbox;

            Fixture fx;
            fx.sink_fn_ = [pjson, &outbox](std::string const & stream) {
                return WebsocketSink::make(
                    [&outbox, stream](std::string t) { outbox[stream].push_back(std::move(t)); },
                    pjson, stream);
            };

            static int s_frame = 7;

            for (auto stream : {"/a", "/b"}) {
                fx.endpoint_map_[stream]
                    = DynamicEndpoint::make_stream(
                          stream,
                          [](rp<WebsocketSink> const &) { return CallbackId(1); },
                          [](CallbackId) {},
                          [](rp<WebsocketSink> const & sink, Json::Value const &) {
                              sink->notify_ev_tp(Reflect::make_tp(&s_frame));
                          });
            }
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");
            router->perform_cmd(R"({"cmd": "send", "stream": "/b", "msg": "step"})");
            router->perform_cmd(R"({"cmd": "send", "stream": "/b", "msg": "step"})");

            REQUIRE(outbox["/a"].empty());
            REQUIRE(outbox["/b"].size() == 2);
            REQUIRE(parse(outbox["/b"][0])["seq"].asInt() == 0);
            REQUIRE(parse(outbox["/b"][1])["seq"].asInt() == 1);
            REQUIRE(parse(outbox["/b"][1])["event"].asInt() == 7);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WsSessionRouter.test.cpp */
