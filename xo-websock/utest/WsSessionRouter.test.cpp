/** @file WsSessionRouter.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  No socket anywhere.  WsSessionRouter reaches the server only through a
 *  UrlRouter (find an endpoint) and the session's WsSender.  Each case
 *  registers endpoints on a REAL UrlRouter, so stream names resolve by the
 *  server's own matching, hands the router a RecordingSender, and drives
 *  perform_cmd() with the exact text a browser would send.  The router makes
 *  REAL sinks on that sender, so what the sender records -- replies and
 *  frames, in order -- is what a browser would receive.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "xo/websock/WsSessionRouter.hpp"
#include "xo/websock/UrlRouter.hpp"
#include "xo/websock/WebsocketSink.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <json/json.h>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace xo {
    using xo::web::WsSessionRouter;
    using xo::web::UrlRouter;
    using xo::web::DynamicEndpoint;
    using xo::web::StreamEndpointDescr;
    using xo::web::StreamReceiver;
    using xo::web::WsSender;
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

            /** everything a router did, as seen from outside it **/
            struct Recorder {
                /* sinks the endpoints' subscribe functions were handed */
                std::vector<rp<WebsocketSink>> subscribed_v_;
                /* (sink, msg) pairs the receivers were handed */
                std::vector<std::pair<rp<WebsocketSink>, Json::Value>> received_v_;
                /* callback ids the unsubscribe functions were handed */
                std::vector<uint32_t> unsubscribed_v_;
                /* everything sent to the session, parsed, in order: replies
                 * (subscribed, unsubscribed, errors) and frames from sinks
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

            /** stands in for a websocket session: keeps each message sent,
             *  in order -- as text, and parsed into @p *p_parsed_v if given
             **/
            class RecordingSender : public WsSender {
            public:
                explicit RecordingSender(std::vector<Json::Value> * p_parsed_v = nullptr)
                    : p_parsed_v_{p_parsed_v} {}

                void send_text(std::string text) override {
                    if (p_parsed_v_)
                        p_parsed_v_->push_back(parse(text));
                    sent_v_.push_back(std::move(text));
                }
                bool is_open() const override { return true; }

                std::vector<std::string> sent_v_;

            private:
                std::vector<Json::Value> * p_parsed_v_ = nullptr;
            };

            /** records each message into a Recorder **/
            class RecordingReceiver : public StreamReceiver {
            public:
                explicit RecordingReceiver(Recorder * rec) : rec_{rec} {}

                void receive(rp<WebsocketSink> const & sink, Json::Value const & msg) override {
                    rec_->received_v_.push_back({sink, msg});
                }

            private:
                Recorder * rec_ = nullptr;
            };

            /** fails every message **/
            class ThrowingReceiver : public StreamReceiver {
            public:
                void receive(rp<WebsocketSink> const &, Json::Value const &) override {
                    throw std::runtime_error("boom");
                }
            };

            /** answers every message with one frame, *p_frame, through the
             *  sender's own sink
             **/
            class FrameReceiver : public StreamReceiver {
            public:
                explicit FrameReceiver(int * p_frame) : p_frame_{p_frame} {}

                void receive(rp<WebsocketSink> const & sink, Json::Value const &) override {
                    sink->notify_ev_tp(Reflect::make_tp(p_frame_));
                }

            private:
                int * p_frame_ = nullptr;
            };

            /** endpoints by stream name, plus a router over them **/
            struct Fixture {
                Recorder rec_;
                /* the real server-side routing; the router borrows it */
                UrlRouter url_router_;

                /* the session: replies and frames, in one order, as on a
                 * socket
                 */
                rp<RecordingSender> sender_{new RecordingSender(&rec_.reply_v_)};
                /* renders frames from the router's (real) sinks */
                rp<PrintJson> pjson_ = PrintJsonSingleton::instance();

                /** an endpoint for @p stream; @p with_receive adds a receiver **/
                void add_endpoint(std::string const & stream, bool with_receive = true) {
                    Recorder * rec = &rec_;

                    auto sub_fn = [rec](rp<WebsocketSink> const & sink) {
                        rec->subscribed_v_.push_back(sink);
                        return CallbackId(static_cast<uint32_t>(rec->subscribed_v_.size()));
                    };
                    auto unsub_fn = [rec](CallbackId id) {
                        rec->unsubscribed_v_.push_back(id.id());
                    };
                    rp<StreamReceiver> receiver;

                    if (with_receive)
                        receiver = new RecordingReceiver(rec);

                    url_router_.register_stream(StreamEndpointDescr(stream, sub_fn, unsub_fn, receiver));
                }

                std::unique_ptr<WsSessionRouter> make_router() {
                    return std::make_unique<WsSessionRouter>(url_router_, sender_, pjson_);
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
            int ev = 5;
            fx.rec_.subscribed_v_[0]->notify_ev_tp(Reflect::make_tp(&ev));

            REQUIRE(fx.rec_.reply_v_.size() == 2);
            REQUIRE(fx.rec_.reply_v_[1]["sub_id"].asUInt() == 0);
            REQUIRE(fx.rec_.reply_v_[1]["event"].asInt() == 5);
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

        TEST_CASE("subscribe-resolves-a-stream-name-through-its-pattern", "[websock][router]")
        {
            /* the server's matching, not an exact-name lookup: one endpoint
             * on a ${var} pattern serves every name under its stem, and the
             * subscription keeps the name the client asked for
             */
            Fixture fx;
            fx.add_endpoint("/fw/${id}");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw/7"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw/8"})");

            REQUIRE(router->n_subscription() == 2);
            REQUIRE(fx.rec_.errors().empty());
            REQUIRE(fx.rec_.subscribed_v_.size() == 2);
            REQUIRE(fx.rec_.subscribed_v_[0]->stream_name() == "/fw/7");
            REQUIRE(fx.rec_.subscribed_v_[1]->stream_name() == "/fw/8");
            REQUIRE(fx.rec_.reply_v_[1]["stream"].asString() == "/fw/8");
            REQUIRE(fx.rec_.reply_v_[1]["sub_id"].asUInt() == 1);

            /* outside the stem: no endpoint */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/other/7"})");

            REQUIRE(fx.rec_.errors().size() == 1);
            REQUIRE(router->n_subscription() == 2);
        }

        TEST_CASE("subscribed-reply-precedes-the-initial-frame", "[websock][router][sink]")
        {
            /* the ordering the protocol depends on: an endpoint whose subscribe
             * function sends a frame at once (the flywheel demo's does) must
             * not get it to the client before the client knows its sub_id.
             * Replies and frames share one ordered log here, as they share one
             * socket in production.
             */
            static int s_initial = 11;

            Fixture fx;
            std::vector<std::string> const & wire = fx.sender_->sent_v_;
            fx.url_router_.register_stream(StreamEndpointDescr(
                "/fw",
                [](rp<WebsocketSink> const & sink) {
                    sink->notify_ev_tp(Reflect::make_tp(&s_initial));
                    return CallbackId(1);
                },
                [](CallbackId) {}));
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
            /* subscribed_v_ is in subscribe order, i.e. by sub_id */
            REQUIRE(fx.rec_.received_v_[0].first.get() == fx.rec_.subscribed_v_[1].get());
            REQUIRE(fx.rec_.received_v_[0].second.asString() == "second");
            REQUIRE(fx.rec_.received_v_[1].first.get() == fx.rec_.subscribed_v_[0].get());
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

            fx.url_router_.register_stream(StreamEndpointDescr(
                "/fw",
                [token](rp<WebsocketSink> const &) { return CallbackId(1); },
                [token, &unsub_log](CallbackId) { unsub_log.push_back("old"); }));

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");

            /* replace: the url router lets go of the old endpoint */
            REQUIRE(fx.url_router_.unregister_stream("/fw"));
            fx.url_router_.register_stream(StreamEndpointDescr(
                "/fw",
                [](rp<WebsocketSink> const &) { return CallbackId(2); },
                [&unsub_log](CallbackId) { unsub_log.push_back("new"); }));

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
            fx.url_router_.register_stream(StreamEndpointDescr(
                "/fw",
                [](rp<WebsocketSink> const &) { return CallbackId(1); },
                [](CallbackId) {},
                new ThrowingReceiver()));
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

        TEST_CASE("removing-an-endpoint-ends-its-subscriptions-only", "[websock][router][removal]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/b");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");   /* sub 0, cb 1 */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");   /* sub 1, cb 2 */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");   /* sub 2, cb 3 */

            /* what unregister will do: the router lets go at once, and the
             * removed endpoint's subscriptions are ended
             */
            rp<DynamicEndpoint> removed = fx.url_router_.find_stream("/a");
            REQUIRE(fx.url_router_.unregister_stream("/a"));

            std::size_t n_reply = fx.rec_.reply_v_.size();

            REQUIRE(router->end_subscriptions_on(removed) == 2);

            /* the removed endpoint's unsubscribe ran once per subscription,
             * in sub_id order; /b's did not run
             */
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{1, 3});
            REQUIRE(router->n_subscription() == 1);

            /* each client told, with the reason */
            REQUIRE(fx.rec_.reply_v_.size() == n_reply + 2);
            for (std::uint32_t k = 0; k < 2; ++k) {
                Json::Value const & r = fx.rec_.reply_v_[n_reply + k];

                REQUIRE(r["cmd"].asString() == "unsubscribed");
                REQUIRE(r["sub_id"].asUInt() == 2 * k);
                REQUIRE(r["reason"].asString() == "endpoint removed");
            }

            /* ended ids are retired, as for a client unsubscribe */
            router->perform_cmd(R"({"cmd": "send", "sub_id": 0, "msg": "step"})");
            REQUIRE(fx.rec_.errors().size() == 1);
            REQUIRE(fx.rec_.errors()[0]["error"].asString() == "already unsubscribed");

            /* /b still works */
            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "still here"})");
            REQUIRE(fx.rec_.received_v_.size() == 1);
            REQUIRE(fx.rec_.received_v_[0].second.asString() == "still here");

            /* a second removal finds nothing; nothing runs twice */
            REQUIRE(router->end_subscriptions_on(removed) == 0);
            REQUIRE(fx.rec_.unsubscribed_v_.size() == 2);
        }

        TEST_CASE("removing-an-endpoint-skips-already-ended-subscriptions", "[websock][router][removal]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");   /* sub 0, cb 1 */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");   /* sub 1, cb 2 */
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 0})");

            std::size_t n_reply = fx.rec_.reply_v_.size();

            REQUIRE(router->end_subscriptions_on(fx.url_router_.find_stream("/a")) == 1);

            /* sub 0's unsubscribe ran once, by the client; only sub 1 ended here */
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{1, 2});
            REQUIRE(fx.rec_.reply_v_.size() == n_reply + 1);
            REQUIRE(fx.rec_.reply_v_[n_reply]["sub_id"].asUInt() == 1);
            REQUIRE(router->n_subscription() == 0);
        }

        TEST_CASE("removing-an-old-endpoint-leaves-its-replacement", "[websock][router][removal]")
        {
            /* identity, not stem: an endpoint unregistered and replaced at the
             * same stem ends only its OWN subscriptions
             */
            Fixture fx;
            fx.add_endpoint("/fw");
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");   /* sub 0, cb 1, old */

            rp<DynamicEndpoint> old_ep = fx.url_router_.find_stream("/fw");
            REQUIRE(fx.url_router_.unregister_stream("/fw"));
            fx.add_endpoint("/fw");

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw"})");   /* sub 1, cb 2, new */

            REQUIRE(router->end_subscriptions_on(old_ep) == 1);
            REQUIRE(fx.rec_.unsubscribed_v_ == std::vector<uint32_t>{1});
            REQUIRE(router->n_subscription() == 1);

            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "new"})");
            REQUIRE(fx.rec_.received_v_.size() == 1);
        }

        TEST_CASE("subscriptions-lists-the-active-ones", "[websock][router]")
        {
            Fixture fx;
            fx.add_endpoint("/a");
            fx.add_endpoint("/fw/${id}");
            auto router = fx.make_router();

            REQUIRE(router->subscriptions().empty());

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");      /* sub 0 */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw/7"})");   /* sub 1 */
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/fw/8"})");   /* sub 2 */
            router->perform_cmd(R"({"cmd": "unsubscribe", "sub_id": 1})");

            auto v = router->subscriptions();

            /* active only, by sub_id; the name asked for, and the serving
             * endpoint's pattern
             */
            REQUIRE(v.size() == 2);
            REQUIRE(v[0].sub_id_ == 0);
            REQUIRE(v[0].stream_name_ == "/a");
            REQUIRE(v[0].endpoint_pattern_ == "/a");
            REQUIRE(v[1].sub_id_ == 2);
            REQUIRE(v[1].stream_name_ == "/fw/8");
            REQUIRE(v[1].endpoint_pattern_ == "/fw/${id}");

            REQUIRE(v.size() == router->n_subscription());
        }

        TEST_CASE("envelope-carries-sub-id-and-per-subscription-seq", "[websock][sink][seq]")
        {
            /* the real sink, handing its finished messages to a recorder: the
             * text here is exactly what a browser would receive
             */
            rp<PrintJson> pjson = PrintJsonSingleton::instance();

            rp<RecordingSender> sender_a(new RecordingSender());
            rp<RecordingSender> sender_b(new RecordingSender());
            std::vector<std::string> const & out_a = sender_a->sent_v_;
            std::vector<std::string> const & out_b = sender_b->sent_v_;

            rp<WebsocketSink> a = WebsocketSink::make(sender_a, pjson, "/a", 4);
            rp<WebsocketSink> b = WebsocketSink::make(sender_b, pjson, "/b", 7);

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
            /* end to end short of a socket: real sinks made by the router, a
             * handler replying through the sink it was handed, and the reply
             * arriving -- enveloped, identified and sequenced -- as frames of
             * that subscription only
             */
            Fixture fx;

            static int s_frame = 7;

            for (auto stream : {"/a", "/b"}) {
                fx.url_router_.register_stream(StreamEndpointDescr(
                    stream,
                    [](rp<WebsocketSink> const &) { return CallbackId(1); },
                    [](CallbackId) {},
                    new FrameReceiver(&s_frame)));
            }
            auto router = fx.make_router();

            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/a"})");
            router->perform_cmd(R"({"cmd": "subscribe", "stream": "/b"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "step"})");
            router->perform_cmd(R"({"cmd": "send", "sub_id": 1, "msg": "step"})");

            /* frames are the messages carrying an "event" */
            std::vector<Json::Value> frame_v;
            for (auto const & m : fx.rec_.reply_v_)
                if (m.isMember("event"))
                    frame_v.push_back(m);

            REQUIRE(frame_v.size() == 2);
            REQUIRE(frame_v[0]["stream"].asString() == "/b");
            REQUIRE(frame_v[0]["sub_id"].asUInt() == 1);
            REQUIRE(frame_v[0]["seq"].asInt() == 0);
            REQUIRE(frame_v[1]["sub_id"].asUInt() == 1);
            REQUIRE(frame_v[1]["seq"].asInt() == 1);
            REQUIRE(frame_v[1]["event"].asInt() == 7);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WsSessionRouter.test.cpp */
