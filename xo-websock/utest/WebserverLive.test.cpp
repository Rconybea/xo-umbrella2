/** @file WebserverLive.test.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  A started Webserver on an OS-assigned port, and a real websocket client
 *  (WsTestClient).  Covers what only runs with libwebsockets: session
 *  bookkeeping, the outbound queue, the service-thread wakeups.  See
 *  .xo-backlog/xo-websock/issues/09.
 *
 *  Kept out of utest.websock, which stays socket-free.
 *
 *  Expectations are OBSERVED, never predicted.
 **/

#include "WsTestClient.hpp"
#include "xo/websock/Webserver.hpp"
#include "xo/websock/WebsocketSink.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <json/json.h>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::StreamEndpointDescr;
    using xo::web::WebsocketSink;
    using xo::web::StreamReceiver;
    using xo::json::PrintJsonSingleton;
    using xo::reflect::Reflect;
    using xo::fn::CallbackId;

    namespace ut {
        namespace {
            using namespace std::chrono_literals;

            /* generous: these only bound a failure, never pace a pass */
            constexpr auto c_timeout = 5s;

            Json::Value parse(std::string const & text) {
                Json::Value root;
                JSONCPP_STRING err;
                std::unique_ptr<Json::CharReader> rd(Json::CharReaderBuilder().newCharReader());

                bool ok = rd->parse(text.data(), text.data() + text.size(), &root, &err);

                INFO("text: " << text << " err: " << err);
                REQUIRE(ok);

                return root;
            }

            /** what a stream endpoint's subscribe function was handed, as
             *  seen from the test's thread (the function runs on the
             *  webserver's)
             **/
            struct SinkBox {
                std::mutex mutex_;
                std::condition_variable cv_;
                std::vector<rp<WebsocketSink>> sink_v_;
                /* callback ids the unsubscribe function was handed */
                std::vector<std::uint32_t> unsub_v_;
                /* messages the receiver was handed */
                std::vector<Json::Value> msg_v_;

                CallbackId subscribe(rp<WebsocketSink> const & sink) {
                    std::lock_guard<std::mutex> lock(mutex_);

                    sink_v_.push_back(sink);
                    cv_.notify_all();

                    return CallbackId(static_cast<uint32_t>(sink_v_.size()));
                }

                void unsubscribe(CallbackId id) {
                    std::lock_guard<std::mutex> lock(mutex_);

                    unsub_v_.push_back(id.id());
                    cv_.notify_all();
                }

                void receive(rp<WebsocketSink> const & sink, Json::Value const & msg) {
                    {
                        std::lock_guard<std::mutex> lock(mutex_);

                        msg_v_.push_back(msg);
                        cv_.notify_all();
                    }

                    /* reply through the sink handed in: reaches exactly the
                     * sender's session.  Rendered synchronously, so a local
                     * is fine
                     */
                    int reply = msg["n"].asInt() * 10;
                    sink->notify_ev_tp(Reflect::make_tp(&reply));
                }

                /* true once at least n unsubscribes have run */
                bool wait_unsubscribed(std::size_t n) {
                    std::unique_lock<std::mutex> lock(mutex_);

                    return cv_.wait_for(lock, c_timeout, [this, n] { return unsub_v_.size() >= n; });
                }

                std::size_t n_unsubscribed() {
                    std::lock_guard<std::mutex> lock(mutex_);

                    return unsub_v_.size();
                }

                /* the n'th sink (0-based) once it exists; null on timeout */
                rp<WebsocketSink> wait_sink(std::size_t n) {
                    std::unique_lock<std::mutex> lock(mutex_);

                    if (!cv_.wait_for(lock, c_timeout, [this, n] { return sink_v_.size() > n; }))
                        return nullptr;

                    return sink_v_[n];
                }
            };

            /** hands each message to a SinkBox, which replies **/
            class BoxReceiver : public StreamReceiver {
            public:
                explicit BoxReceiver(std::shared_ptr<SinkBox> box) : box_{std::move(box)} {}

                void receive(rp<WebsocketSink> const & sink, Json::Value const & msg) override {
                    box_->receive(sink, msg);
                }

            private:
                std::shared_ptr<SinkBox> box_;
            };

            /** stream endpoint on @p pattern, reporting to @p box **/
            StreamEndpointDescr box_descr(std::string pattern, std::shared_ptr<SinkBox> const & box) {
                return StreamEndpointDescr(std::move(pattern),
                                           [box](rp<WebsocketSink> const & sink) { return box->subscribe(sink); },
                                           [box](CallbackId id) { box->unsubscribe(id); },
                                           new BoxReceiver(box));
            }

            /** true once @p pred holds; polls, since the server's session
             *  bookkeeping runs on its own thread with nothing to wait on
             **/
            template <typename Pred>
            bool wait_until(Pred pred) {
                auto deadline = std::chrono::steady_clock::now() + c_timeout;

                while (!pred()) {
                    if (std::chrono::steady_clock::now() > deadline)
                        return false;
                    std::this_thread::sleep_for(1ms);
                }

                return true;
            }

            /** a started webserver on an OS-assigned port **/
            struct LiveServer {
                LiveServer() {
                    websrv_ = Webserver::make(WebserverConfig(), PrintJsonSingleton::instance());
                }

                ~LiveServer() {
                    websrv_->stop_webserver();
                    websrv_->join_webserver();
                }

                /* start; the port once listening, 0 on timeout */
                std::int32_t start() {
                    websrv_->start_webserver();

                    auto deadline = std::chrono::steady_clock::now() + c_timeout;

                    /* start_webserver() returns before the service thread has
                     * bound the port; nothing to wait on but the port itself
                     */
                    while (websrv_->listen_port() == 0) {
                        if (std::chrono::steady_clock::now() > deadline)
                            return 0;
                        std::this_thread::sleep_for(1ms);
                    }

                    return websrv_->listen_port();
                }

                rp<Webserver> websrv_;
            };
        }

        TEST_CASE("live-subscribe-then-frames", "[websock][live]")
        {
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint
                (StreamEndpointDescr("/fw",
                                     [box](rp<WebsocketSink> const & sink) { return box->subscribe(sink); },
                                     [](CallbackId) {}));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            WsTestClient client(port);
            REQUIRE(client.wait_connected(c_timeout));

            client.send(R"({"cmd": "subscribe", "stream": "/fw"})");

            REQUIRE(client.wait_received(1, c_timeout));
            Json::Value subscribed = parse(client.received()[0]);
            REQUIRE(subscribed["cmd"].asString() == "subscribed");
            REQUIRE(subscribed["stream"].asString() == "/fw");
            std::uint32_t sub_id = subscribed["sub_id"].asUInt();

            /* the application's side: events pushed from its own thread, as
             * a reactor source would
             */
            rp<WebsocketSink> sink = box->wait_sink(0);
            REQUIRE(sink);

            int ev1 = 101;
            int ev2 = 102;
            sink->notify_ev_tp(Reflect::make_tp(&ev1));
            sink->notify_ev_tp(Reflect::make_tp(&ev2));

            REQUIRE(client.wait_received(3, c_timeout));
            std::vector<std::string> msg_v = client.received();

            for (std::size_t i = 1; i < 3; ++i) {
                Json::Value env = parse(msg_v[i]);

                INFO("frame: " << msg_v[i]);
                REQUIRE(env["stream"].asString() == "/fw");
                REQUIRE(env["sub_id"].asUInt() == sub_id);
                REQUIRE(env["seq"].asInt() == static_cast<int>(i - 1));
                REQUIRE(env["event"].asInt() == 100 + static_cast<int>(i));
            }

            REQUIRE(client.close(c_timeout));
        }

        TEST_CASE("live-a-server-that-cannot-start-still-joins", "[websock][live]")
        {
            /* lws_create_context fails when the port is taken; run() used to
             * return without reporting stopped, so join_webserver() hung
             */
            LiveServer first;
            std::int32_t port = first.start();
            REQUIRE(port > 0);

            rp<Webserver> second = Webserver::make(WebserverConfig(port, false, false, false),
                                                   PrintJsonSingleton::instance());
            second->start_webserver();

            /* join on a DETACHED thread, so a regression fails instead of
             * hanging: a std::async future would block in its destructor.
             * The thread holds its own rp, so on regression neither it nor
             * ~WebserverImpl (which also joins) runs on the test's thread.
             */
            std::promise<void> joined_promise;
            std::future<void> joined = joined_promise.get_future();

            std::thread([second, p = std::move(joined_promise)]() mutable
                {
                    second->join_webserver();
                    p.set_value();
                }).detach();

            REQUIRE(joined.wait_for(c_timeout) == std::future_status::ready);
            REQUIRE(second->listen_port() == 0);
            REQUIRE(second->state() == xo::web::Runstate::stopped);
        }

        TEST_CASE("live-send-reaches-the-receiver-and-its-reply-comes-back", "[websock][live]")
        {
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint(box_descr("/fw", box));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            WsTestClient client(port);
            REQUIRE(client.wait_connected(c_timeout));

            client.send(R"({"cmd": "subscribe", "stream": "/fw"})");
            REQUIRE(client.wait_received(1, c_timeout));
            std::uint32_t sub_id = parse(client.received()[0])["sub_id"].asUInt();

            client.send(std::string(R"({"cmd": "send", "sub_id": )")
                        + std::to_string(sub_id)
                        + R"(, "msg": {"op": "step", "n": 4}})");

            /* the receiver's reply, as a frame of this subscription */
            REQUIRE(client.wait_received(2, c_timeout));
            Json::Value frame = parse(client.received()[1]);

            REQUIRE(frame["sub_id"].asUInt() == sub_id);
            REQUIRE(frame["seq"].asInt() == 0);
            REQUIRE(frame["event"].asInt() == 40);

            /* and the receiver saw the msg exactly as sent */
            std::lock_guard<std::mutex> lock(box->mutex_);
            REQUIRE(box->msg_v_.size() == 1);
            REQUIRE(box->msg_v_[0]["op"].asString() == "step");
            REQUIRE(box->msg_v_[0]["n"].asInt() == 4);
        }

        TEST_CASE("live-unregister-ends-a-live-subscription", "[websock][live]")
        {
            /* issue 07 over a socket: the unregister queue, the service
             * thread's wakeup and drain, and the reply reaching the client
             */
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint(box_descr("/fw/${id}", box));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            WsTestClient client(port);
            REQUIRE(client.wait_connected(c_timeout));

            client.send(R"({"cmd": "subscribe", "stream": "/fw/1"})");
            client.send(R"({"cmd": "subscribe", "stream": "/fw/2"})");
            REQUIRE(client.wait_received(2, c_timeout));
            REQUIRE(box->wait_sink(1));

            /* from the test's thread, as python would */
            REQUIRE(srv.websrv_->unregister_stream_endpoint("/fw/${id}"));

            REQUIRE(client.wait_received(4, c_timeout));
            std::vector<std::string> msg_v = client.received();

            for (std::size_t i = 2; i < 4; ++i) {
                Json::Value r = parse(msg_v[i]);

                INFO("reply: " << msg_v[i]);
                REQUIRE(r["cmd"].asString() == "unsubscribed");
                REQUIRE(r["sub_id"].asUInt() == i - 2);
                REQUIRE(r["reason"].asString() == "endpoint removed");
            }

            /* the endpoint's unsubscribe ran once per subscription -- a real
             * source would stop pushing here
             */
            REQUIRE(box->wait_unsubscribed(2));

            /* a new subscribe finds nothing */
            client.send(R"({"cmd": "subscribe", "stream": "/fw/3"})");
            REQUIRE(client.wait_received(5, c_timeout));
            REQUIRE(parse(client.received()[4])["error"].asString() == "unknown stream");

            /* and nothing ran twice */
            REQUIRE(box->n_unsubscribed() == 2);
        }

        TEST_CASE("live-a-sink-kept-past-its-session-reaches-no-one", "[websock][live]")
        {
            /* issues 05 and 08: the kept sink's sender is closed with its
             * session, and the session id is never reused, so a later client
             * gets nothing from it
             */
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint(box_descr("/fw", box));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            rp<WebsocketSink> kept;

            {
                WsTestClient first(port);
                REQUIRE(first.wait_connected(c_timeout));

                first.send(R"({"cmd": "subscribe", "stream": "/fw"})");
                REQUIRE(first.wait_received(1, c_timeout));

                kept = box->wait_sink(0);
                REQUIRE(kept);

                REQUIRE(first.close(c_timeout));
            }

            /* the server has handled the close once the session's
             * subscriptions are unsubscribed (notify_ws_session_close)
             */
            REQUIRE(box->wait_unsubscribed(1));

            WsTestClient second(port);
            REQUIRE(second.wait_connected(c_timeout));

            /* the application pushes to the sink it kept */
            int stale = 666;
            kept->notify_ev_tp(Reflect::make_tp(&stale));

            /* barrier: the second client's own traffic.  Had the stale frame
             * been delivered it would precede this reply -- a closed sender
             * drops synchronously, before this subscribe is even sent
             */
            second.send(R"({"cmd": "subscribe", "stream": "/fw"})");
            REQUIRE(second.wait_received(1, c_timeout));

            std::vector<std::string> msg_v = second.received();

            INFO("first message: " << msg_v[0]);
            REQUIRE(parse(msg_v[0])["cmd"].asString() == "subscribed");
            for (auto const & m : msg_v)
                REQUIRE(m.find("666") == std::string::npos);
        }

        TEST_CASE("live-sessions-lists-each-connection", "[websock][live]")
        {
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint(box_descr("/fw", box));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            REQUIRE(srv.websrv_->sessions().empty());

            auto first = std::make_unique<WsTestClient>(port);
            REQUIRE(first->wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return srv.websrv_->sessions().size() == 1; }));

            WsTestClient second(port);
            REQUIRE(second.wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return srv.websrv_->sessions().size() == 2; }));

            /* the second subscribes; the reply precedes the subscribe
             * function, so wait on the sink, not the reply
             */
            second.send(R"({"cmd": "subscribe", "stream": "/fw"})");
            REQUIRE(second.wait_received(1, c_timeout));
            REQUIRE(box->wait_sink(0));

            auto v = srv.websrv_->sessions();

            /* by id, in connection order; distinct; both open */
            REQUIRE(v.size() == 2);
            REQUIRE(v[0].session_id_ < v[1].session_id_);
            REQUIRE(v[0].sender_open_);
            REQUIRE(v[1].sender_open_);
            REQUIRE(v[0].subscriptions_.empty());
            REQUIRE(v[1].subscriptions_.size() == 1);
            REQUIRE(v[1].subscriptions_[0].stream_name_ == "/fw");
            REQUIRE(v[1].subscriptions_[0].endpoint_pattern_ == "/fw");

            std::uint64_t second_id = v[1].session_id_;

            /* the same state, through the Webserver json printer */
            {
                Webserver * server = srv.websrv_.get();
                std::stringstream ss;
                PrintJsonSingleton::instance()->print(server, &ss);

                Json::Value const sessions = parse(ss.str())["sessions"];

                INFO("json: " << ss.str());
                REQUIRE(sessions.size() == 2);
                REQUIRE(sessions[0]["id"].asUInt64() == v[0].session_id_);
                REQUIRE(sessions[1]["sender_open"].asBool());
                REQUIRE(sessions[1]["subscriptions"].size() == 1);
                REQUIRE(sessions[1]["subscriptions"][0]["stream"].asString() == "/fw");
            }

            /* a closed session leaves the listing */
            REQUIRE(first->close(c_timeout));
            first.reset();

            REQUIRE(wait_until([&] { return srv.websrv_->sessions().size() == 1; }));
            REQUIRE(srv.websrv_->sessions()[0].session_id_ == second_id);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WebserverLive.test.cpp */
