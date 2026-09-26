/** @file WsSessionRouter.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
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
#include <iterator>
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
                FakeSink(std::string stream, uint32_t sub_id)
                    : stream_{std::move(stream)}, sub_id_{sub_id} {}

                std::string const & stream_name() const override { return stream_; }
                uint32_t n_in_ev() const override { return n_in_ev_; }
                void notify_ev_tp(TaggedPtr const &) override { ++n_in_ev_; }
                void pretty(xo::pp::PpSink & sink) const override { sink.put("<FakeSink>"); }
                std::string display_string() const override { return "<FakeSink>"; }

                std::string stream_;
                uint32_t sub_id_ = 0;
                uint32_t n_in_ev_ = 0;
            };

            uint32_t sub_id_of(rp<WebsocketSink> const & sink) {
                return dynamic_cast<FakeSink &>(*sink.get()).sub_id_;
            }

            /** everything a router did, as seen from outside it **/
            struct Recorder {
                /* sinks the endpoints' subscribe functions were handed */
                std::vector<rp<WebsocketSink>> subscribed_v_;
                /* (sink, msg) pairs the receive functions were handed */
                std::vector<std::pair<rp<WebsocketSink>, Json::Value>> received_v_;
                /* callback ids the unsubscribe functions were handed */
                std::vector<uint32_t> unsubscribed_v_;
                /* every reply outside a subscription, parsed: subscribed,
                 * unsubscribed and errors, in order
                 */
                std::vector<Json::Value> reply_v_;

                std::vector<Json::Value> errors() const {
                    std::vector<Json::Value> v;
                    for (auto const & r : reply_v_)
                        if (r.isMember("error"))
                            v.push_back(r);
                    return v;
                }
            };

            /** endpoints by stream name, plus a router over them **/
            struct Fixture {
                Recorder rec_;
                std::map<std::string, rp<DynamicEndpoint>> endpoint_map_;

                /* how the router makes a sink.  FakeSink unless a case needs
                 * the real envelope
                 */
                WsSessionRouter::SinkFactory sink_fn_
                    = [](std::string const & stream, uint32_t sub_id) -> rp<WebsocketSink> {
                          return new FakeSink(stream, sub_id);
                      };

                /* replies go here; a case can redirect them */
                WsSessionRouter::ReplyFn reply_fn_;

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

                    WsSessionRouter::ReplyFn reply_fn = reply_fn_;
                    if (!reply_fn) {
                        reply_fn = [rec](std::string text) {
                            rec->reply_v_.push_back(parse(text));
                        };
                    }

                    return std::make_unique<WsSessionRouter>(
                        [epmap](std::string const & stream) -> DynamicEndpoint * {
                            auto ix = epmap->find(stream);
                            return (ix == epmap->end()) ? nullptr : ix->second.get();
                        },
                        sink_fn_,
                        reply_fn);
                }
            };
        } /*namespace*/

        TEST_CASE("subscribe-answers-with-a-server-assigned-sub-id", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE(fx.rec_.subscribed_v_.size() == 1);
            REQUIRE(fx.rec_.subscribed_v_[0]->stream_name() == "/fw");
            REQUIRE(router->n_subscription() == 1);

            REQUIRE(fx.rec_.reply_v_.size() == 1);
            Json::Value const & r = fx.rec_.reply_v_[0];
            REQUIRE(r["cmd"].asString() == "subscribed");
            REQUIRE(r["stream"].asString() == "/fw");
            REQUIRE(r["sub_id"].asUInt() == 0);

            /* the sink carries the same id the client was told */
            REQUIRE(sub_id_of(fx.rec_.subscribed_v_[0]) == 0);
        }

        TEST_CASE("subscribe-to-an-unknown-stream-is-an-error", "[websock][router]")
        {
            /* silent until issue 06 (and pinned that way by issue 04's
             * subscribe-to-an-unknown-stream-stays-silent); a page subscribing
             * to a stream that does not exist now hears about it
             */
            Fixture fx;
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/nope"})");

            REQUIRE(router->n_subscription() == 0);
            REQUIRE(fx.rec_.reply_v_.size() == 1);
            REQUIRE(fx.rec_.reply_v_[0]["error"].asString() == "unknown stream");
            REQUIRE(fx.rec_.reply_v_[0]["stream"].asString() == "/nope");
        }

        TEST_CASE("subscribed-reply-precedes-the-initial-frame", "[websock][router][sink]")
        {
            /* the ordering the protocol depends on: an endpoint whose subscribe
             * function sends a frame at once (the flywheel demo's does) must
             * not get it to the client before the client knows its sub_id.
             * Replies and frames share one ordered log here, as they share one
             * socket in production.
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();
            std::vector<std::string> wire;
            static int s_initial = 11;

            Fixture fx;
            fx.reply_fn_ = [&wire](std::string t) { wire.push_back(std::move(t)); };
            fx.sink_fn_ = [pjson, &wire](std::string const & stream, uint32_t sub_id) {
                return WebsocketSink::make(
                    [&wire](std::string t) { wire.push_back(std::move(t)); },
                    pjson, stream, sub_id);
            };
            fx.endpoint_map_["/fw"]
                = DynamicEndpoint::make_stream(
                      "/fw",
                      [](rp<WebsocketSink> const & sink) {
                          sink->notify_ev_tp(Reflect::make_tp(&s_initial));
                          return CallbackId(1);
                      },
                      [](CallbackId) {});
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE(wire.size() == 2);
            REQUIRE(parse(wire[0])["cmd"].asString() == "subscribed");
            REQUIRE(parse(wire[0])["sub_id"].asUInt() == 0);
            REQUIRE(parse(wire[1])["sub_id"].asUInt() == 0);
            REQUIRE(parse(wire[1])["event"].asInt() == 11);
        }

        TEST_CASE("send-reaches-receive-with-the-subscriptions-own-sink", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": {"op": "step", "n": 2}})");

            REQUIRE(fx.rec_.received_v_.size() == 1);
            REQUIRE(fx.rec_.received_v_[0].first.get() == fx.rec_.subscribed_v_[0].get());

            Json::Value const & msg = fx.rec_.received_v_[0].second;

            REQUIRE(msg["op"].asString() == "step");
            REQUIRE(msg["n"].asInt() == 2);
            REQUIRE(fx.rec_.errors().empty());
        }

        TEST_CASE("one-stream-subscribed-twice-is-addressed-by-sub-id", "[websock][router]")
        {
            /* what issue 06 exists for: under issue 04's stream-name
             * addressing the second subscription was unreachable
             */
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE(fx.rec_.reply_v_[1]["sub_id"].asUInt() == 1);

            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "second"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "first"})");

            REQUIRE(fx.rec_.received_v_.size() == 2);
            REQUIRE(sub_id_of(fx.rec_.received_v_[0].first) == 1);
            REQUIRE(fx.rec_.received_v_[0].second.asString() == "second");
            REQUIRE(sub_id_of(fx.rec_.received_v_[1].first) == 0);
            REQUIRE(fx.rec_.received_v_[1].second.asString() == "first");
        }

        TEST_CASE("send-to-an-unknown-sub-id-is-an-error", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "step"})");

            REQUIRE(fx.rec_.received_v_.empty());
            REQUIRE(fx.rec_.reply_v_.size() == 1);
            REQUIRE(fx.rec_.reply_v_[0]["error"].asString() == "unknown sub_id");
            REQUIRE(fx.rec_.reply_v_[0]["sub_id"].asUInt() == 0);
        }

        TEST_CASE("send-to-a-stream-without-a-receive-fn-is-an-error", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw", false /*with_receive*/);
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "step"})");

            auto errors = fx.rec_.errors();
            REQUIRE(errors.size() == 1);
            REQUIRE(errors[0]["error"].asString() == "stream does not accept messages");
            REQUIRE(errors[0]["stream"].asString() == "/fw");
            REQUIRE(errors[0]["sub_id"].asUInt() == 0);
        }

        TEST_CASE("unsubscribe-by-sub-id", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 0})");

            /* the endpoint's unsubscribe ran, with its own callback id */
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{1});
            REQUIRE(router->n_subscription() == 0);

            REQUIRE(fx.rec_.reply_v_.size() == 2);
            REQUIRE(fx.rec_.reply_v_[1]["cmd"].asString() == "unsubscribed");
            REQUIRE(fx.rec_.reply_v_[1]["sub_id"].asUInt() == 0);

            /* and the id is now dead: not unknown, retired */
            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "step"})");
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 0})");

            auto errors = fx.rec_.errors();
            REQUIRE(errors.size() == 2);
            REQUIRE(errors[0]["error"].asString() == "already unsubscribed");
            REQUIRE(errors[1]["error"].asString() == "already unsubscribed");
            REQUIRE(fx.rec_.received_v_.empty());
            /* unsubscribing twice does not run the endpoint's unsubscribe twice */
            REQUIRE(fx.rec_.unsubscribed_v_.size() == 1);
        }

        TEST_CASE("an-endpoint-replaced-while-subscribed-outlives-the-map", "[websock][router][ownership]")
        {
            /* the hazard DynamicEndpoint's refcount removes.  Before, the map
             * held the only owner (unique_ptr) and a subscription a raw
             * pointer, so re-registering a stem freed the endpoint under a live
             * subscription, whose unsubscribe then called into freed memory.
             *
             * `token' is captured by the OLD endpoint's functions only, so its
             * use_count shows whether that endpoint is still alive.
             */
            Fixture fx;
            auto router = fx.make_router();

            auto token = std::make_shared<int>(0);
            std::vector<std::string> unsub_log;

            fx.endpoint_map_["/fw"]
                = DynamicEndpoint::make_stream(
                      "/fw",
                      [token](rp<WebsocketSink> const &) { return CallbackId(1); },
                      [token, &unsub_log](CallbackId) { unsub_log.push_back("old"); });

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            /* re-register: the map lets go of the old endpoint */
            fx.endpoint_map_["/fw"]
                = DynamicEndpoint::make_stream(
                      "/fw",
                      [](rp<WebsocketSink> const &) { return CallbackId(2); },
                      [&unsub_log](CallbackId) { unsub_log.push_back("new"); });

            /* ...but the subscription has not: the old endpoint is alive */
            REQUIRE(token.use_count() > 1);

            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 0})");

            /* unsubscribe ran on the endpoint that subscribed us, not its
             * replacement -- and was then the last owner
             */
            REQUIRE(unsub_log == std::vector<std::string>{"old"});
            REQUIRE(token.use_count() == 1);
        }

        TEST_CASE("a-retired-sub-id-is-never-reused", "[websock][router]")
        {
            /* the stale-id hazard: if subscription 0's slot were reused, a page
             * still holding id 0 would steer messages into someone else's
             * subscription
             */
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/b");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 0})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");

            /* /b got a FRESH id */
            REQUIRE(fx.rec_.reply_v_[2]["cmd"].asString() == "subscribed");
            REQUIRE(fx.rec_.reply_v_[2]["sub_id"].asUInt() == 1);

            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "stale"})");

            REQUIRE(fx.rec_.received_v_.empty());
            REQUIRE(fx.rec_.errors().size() == 1);
            REQUIRE(fx.rec_.errors()[0]["error"].asString() == "already unsubscribed");
        }

        TEST_CASE("bad-messages-get-an-error-and-never-throw", "[websock][router]")
        {
            /* a json string or array crashed the server before issue 04:
             * jsoncpp throws from operator[] on a non-object, and the
             * exception escaped into libwebsockets' C callback
             */
            Fixture fx;
            auto router = fx.make_router();

            char const * bad[] = {
                "not json at all",
                R"("a bare string")",
                R"([1, 2, 3])",
                R"({"cmd": "subscribe"})",
                R"({"cmd": "send"})",
                R"({"cmd": "send", "sub_id": -1})",
                R"({"cmd": "send", "sub_id": "0"})",
                R"({"cmd": "send", "sub_id": 1.5})",
                R"({"cmd": "unsubscribe"})",
            };

            for (char const * msg : bad) {
                INFO("message: " << msg);
                REQUIRE_NOTHROW(router->perform_cmd(msg));
            }

            auto errors = fx.rec_.errors();
            REQUIRE(errors.size() == std::size(bad));
            for (auto const & e : errors) {
                INFO("error: " << e["error"].asString());
                /* none of these can name a stream or a valid sub_id */
                REQUIRE(!e.isMember("stream"));
                REQUIRE(!e.isMember("sub_id"));
            }

            /* a non-string cmd is an unknown command: ignored, as before */
            REQUIRE_NOTHROW(router->perform_cmd(R"({"cmd": 3, "stream": "/fw"})"));
            REQUIRE(fx.rec_.errors().size() == std::size(bad));
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

            REQUIRE_NOTHROW(router->perform_cmd(R"({"cmd": "send", "sub_id": 0})"));
            REQUIRE(fx.rec_.errors().size() == 1);
            REQUIRE(fx.rec_.errors()[0]["error"].asString() == "stream handler failed: boom");
        }

        TEST_CASE("unsubscribe-all-releases-every-active-subscription", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/b");
            fx.add_endpoint("/c");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/c"})");
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 1})");

            router->unsubscribe_all();

            /* /b once, by its own unsubscribe -- not again for the retired slot */
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{2, 1, 3});
            REQUIRE(router->n_subscription() == 0);
        }

        TEST_CASE("envelope-carries-sub-id-and-per-subscription-seq", "[websock][sink][seq]")
        {
            /* the real sink, handing its finished messages to a recorder: the
             * text here is exactly what a browser would receive
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();

            std::vector<std::string> out_a;
            std::vector<std::string> out_b;

            rp<WebsocketSink> a = WebsocketSink::make(
                [&out_a](std::string t) { out_a.push_back(std::move(t)); }, pjson, "/a", 4);
            rp<WebsocketSink> b = WebsocketSink::make(
                [&out_b](std::string t) { out_b.push_back(std::move(t)); }, pjson, "/b", 7);

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
                REQUIRE(env["sub_id"].asUInt() == 4);
                REQUIRE(env["seq"].asInt() == i);
                REQUIRE(env["event"].asInt() == 42);
            }

            /* b's count is its own: interleaving with a does not advance it */
            REQUIRE(parse(out_b[0])["sub_id"].asUInt() == 7);
            REQUIRE(parse(out_b[0])["seq"].asInt() == 0);
            REQUIRE(a->n_in_ev() == 3);
        }

        TEST_CASE("a-handlers-reply-goes-out-through-its-subscription", "[websock][router][sink]")
        {
            /* end to end short of a socket: real sinks from the router's sink
             * factory, a handler replying through the sink it was handed, and
             * the reply arriving -- enveloped, identified and sequenced -- on
             * that subscription's outbox only
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();
            std::map<std::string, std::vector<std::string>> outbox;

            Fixture fx;
            fx.sink_fn_ = [pjson, &outbox](std::string const & stream, uint32_t sub_id) {
                return WebsocketSink::make(
                    [&outbox, stream](std::string t) { outbox[stream].push_back(std::move(t)); },
                    pjson, stream, sub_id);
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
            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "step"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "step"})");

            REQUIRE(outbox["/a"].empty());
            REQUIRE(outbox["/b"].size() == 2);
            REQUIRE(parse(outbox["/b"][0])["sub_id"].asUInt() == 1);
            REQUIRE(parse(outbox["/b"][0])["seq"].asInt() == 0);
            REQUIRE(parse(outbox["/b"][1])["seq"].asInt() == 1);
            REQUIRE(parse(outbox["/b"][1])["event"].asInt() == 7);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WsSessionRouter.test.cpp */
