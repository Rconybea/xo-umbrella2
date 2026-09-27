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
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::StreamEndpointDescr;
    using xo::web::WebsocketSink;
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

                CallbackId subscribe(rp<WebsocketSink> const & sink) {
                    std::lock_guard<std::mutex> lock(mutex_);

                    sink_v_.push_back(sink);
                    cv_.notify_all();

                    return CallbackId(static_cast<uint32_t>(sink_v_.size()));
                }

                /* the n'th sink (0-based) once it exists; null on timeout */
                rp<WebsocketSink> wait_sink(std::size_t n) {
                    std::unique_lock<std::mutex> lock(mutex_);

                    if (!cv_.wait_for(lock, c_timeout, [this, n] { return sink_v_.size() > n; }))
                        return nullptr;

                    return sink_v_[n];
                }
            };

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
    } /*namespace ut*/
} /*namespace xo*/

/* end WebserverLive.test.cpp */
