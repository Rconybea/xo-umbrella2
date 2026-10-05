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
#include "WebsockUtestAppcx.hpp"
#include "xo/websock/Webserver.hpp"
#include "xo/websock/WebsocketSink.hpp"
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/reflect/Reflect.hpp>
#include <catch2/catch.hpp>
#include <json/json.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
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
    using xo::web::HttpEndpointDescr;
    using xo::web::HttpRequest;
    using xo::web::HttpResponse;
    using xo::web::HttpStatus;
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

                /* a StreamReceiver is SelfTagging; not reflected in full */
                xo::reflect::TaggedRcptr self_tp() override { return Reflect::make_rctp(this); }

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

            /** one http GET: status line code, Content-Type, body **/
            struct HttpReply {
                int status_ = 0;
                std::string content_type_;
                std::string body_;
            };

            /** GET @p path from localhost:@p port, over HTTP/1.0 (so the
             *  server closes when done).  status_ 0 if no reply
             **/
            HttpReply http_get(std::int32_t port, std::string const & path) {
                HttpReply reply;

                int fd = ::socket(AF_INET, SOCK_STREAM, 0);
                if (fd < 0)
                    return reply;

                /* bound a hung server */
                timeval tv{5, 0};
                ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

                sockaddr_in addr{};
                addr.sin_family = AF_INET;
                addr.sin_port = htons(static_cast<std::uint16_t>(port));
                addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

                if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
                    ::close(fd);
                    return reply;
                }

                std::string req = "GET " + path + " HTTP/1.0\r\nHost: localhost\r\n\r\n";
                ::send(fd, req.data(), req.size(), 0);

                std::string text;
                char buf[4096];
                for (ssize_t n; (n = ::recv(fd, buf, sizeof(buf), 0)) > 0; )
                    text.append(buf, n);
                ::close(fd);

                auto hdr_end = text.find("\r\n\r\n");
                if (hdr_end == std::string::npos)
                    return reply;

                std::string head = text.substr(0, hdr_end);
                reply.body_ = text.substr(hdr_end + 4);

                /* "HTTP/1.x NNN ..." */
                auto sp = head.find(' ');
                if (sp != std::string::npos)
                    reply.status_ = std::atoi(head.c_str() + sp + 1);

                /* header names are case-insensitive: lws sends lowercase */
                std::string lower = head;
                for (char & c : lower)
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                auto ct = lower.find("\r\ncontent-type:");
                if (ct != std::string::npos) {
                    auto v = ct + std::string("\r\ncontent-type:").size();
                    auto e = head.find("\r\n", v);
                    reply.content_type_ = head.substr(v, e - v);
                    while (!reply.content_type_.empty() && reply.content_type_.front() == ' ')
                        reply.content_type_.erase(0, 1);
                }

                return reply;
            }

            /** a started webserver on an OS-assigned port **/
            struct LiveServer {
                LiveServer() {
                    websrv_ = Webserver::make(WebsockUtestAppcx::appcx().cx<S_websock_tag>(), WebserverConfig());
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

        TEST_CASE("live-http-status-and-content-type", "[websock][live][http]")
        {
            /* what a handler chooses is what the client gets: status line and
             * Content-Type header; and the server's own answers -- no
             * endpoint, no match, a handler's exception
             */
            LiveServer srv;
            srv.websrv_->register_http_endpoint
                (HttpEndpointDescr("/j",
                                   [](HttpRequest const &) { return HttpResponse::json("{\"a\": 1}"); }));
            srv.websrv_->register_http_endpoint
                (HttpEndpointDescr("/h/${a}",
                                   [](HttpRequest const & req) {
                                       return HttpResponse::html("<p>" + std::string(req.var("a")) + "</p>");
                                   }));
            srv.websrv_->register_http_endpoint
                (HttpEndpointDescr("/boom",
                                   [](HttpRequest const &) -> HttpResponse {
                                       throw std::runtime_error("kaboom");
                                   }));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            HttpReply j = http_get(port, "/dyn/j");
            REQUIRE(j.status_ == 200);
            REQUIRE(j.content_type_ == "application/json");
            REQUIRE(j.body_ == "{\"a\": 1}");

            HttpReply h = http_get(port, "/dyn/h/x-1.hpp");
            REQUIRE(h.status_ == 200);
            REQUIRE(h.content_type_ == "text/html; charset=utf-8");
            REQUIRE(h.body_ == "<p>x-1.hpp</p>");

            /* found by stem, pattern not matched */
            HttpReply nm = http_get(port, "/dyn/h/x/y");
            REQUIRE(nm.status_ == 404);
            REQUIRE(nm.content_type_ == "text/html; charset=utf-8");

            /* no endpoint at all */
            HttpReply ne = http_get(port, "/dyn/nothing-here");
            REQUIRE(ne.status_ == 404);

            /* a handler's exception: 500, and the server lives on */
            HttpReply b = http_get(port, "/dyn/boom");
            REQUIRE(b.status_ == 500);
            REQUIRE(b.body_.find("kaboom") != std::string::npos);

            REQUIRE(http_get(port, "/dyn/j").status_ == 200);
        }

        TEST_CASE("live-a-server-that-cannot-start-still-joins", "[websock][live]")
        {
            /* lws_create_context fails when the port is taken; run() used to
             * return without reporting stopped, so join_webserver() hung
             */
            LiveServer first;
            std::int32_t port = first.start();
            REQUIRE(port > 0);

            rp<Webserver> second = Webserver::make(WebsockUtestAppcx::appcx().cx<S_websock_tag>(),
                                                   WebserverConfig(port, false, false, false));
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

            /* the server as its json printer shows it */
            auto server_json = [&srv] {
                Webserver * server = srv.websrv_.get();
                std::stringstream ss;
                PrintJsonSingleton::instance()->print(server, &ss);
                return parse(ss.str());
            };
            auto n_session = [&] { return server_json()["sessions"].size(); };

            REQUIRE(n_session() == 0);

            auto first = std::make_unique<WsTestClient>(port);
            REQUIRE(first->wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return n_session() == 1; }));

            WsTestClient second(port);
            REQUIRE(second.wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return n_session() == 2; }));

            /* the second subscribes; the reply precedes the subscribe
             * function, so wait on the sink, not the reply
             */
            second.send(R"({"cmd": "subscribe", "stream": "/fw"})");
            REQUIRE(second.wait_received(1, c_timeout));
            REQUIRE(box->wait_sink(0));

            Json::Value const root = server_json();
            Json::Value const & v = root["sessions"];

            INFO("json: " << root.toStyledString());

            /* by id, in connection order; distinct */
            REQUIRE(v.size() == 2);
            REQUIRE(v[0]["_name_"].asString() == "WsSession");
            REQUIRE(v[0]["_canonical_type_"].asString() == "xo::web::WebsocketSessionRecd");
            REQUIRE(v[0]["session_id"].asUInt64() < v[1]["session_id"].asUInt64());
            REQUIRE(v[0]["_id_"].asInt() != v[1]["_id_"].asInt());

            /* each session's sender, in full: open; its session's id; held
             * by the session record and the router, plus one per sink
             */
            for (Json::ArrayIndex k = 0; k < 2; ++k) {
                Json::Value const & sender = v[k]["sender"];

                REQUIRE(sender["_name_"].asString() == "WsSessionSender");

                /* its chosen C++ members: target_ a ref to the server */
                Json::Value const & smem = sender["_members_"];
                REQUIRE(smem[0]["_name_"].asString() == "target_");
                REQUIRE(smem[0]["_value_"]["_ref_"].asInt() == root["_id_"].asInt());
                REQUIRE(smem[1]["_name_"].asString() == "session_id_");
                REQUIRE(smem[1]["_value_"].asUInt64() == v[k]["session_id"].asUInt64());
                REQUIRE(smem[2]["_name_"].asString() == "open_");
                REQUIRE(smem[2]["_value_"].asBool());
                /* a template: its arguments follow */
                REQUIRE(sender["_canonical_type_"].asString().starts_with("xo::web::WsSessionSender<"));
                REQUIRE(sender["_short_type_"].asString().starts_with("WsSessionSender<"));
                REQUIRE(sender["open"].asBool());
                REQUIRE(sender["session_id"].asUInt64() == v[k]["session_id"].asUInt64());
            }
            REQUIRE(v[0]["sender"]["refcount"].asUInt() == 2);
            REQUIRE(v[1]["sender"]["refcount"].asUInt() == 3);

            REQUIRE(v[0]["subscriptions"].empty());
            REQUIRE(v[1]["subscriptions"].size() == 1);
            REQUIRE(v[1]["subscriptions"][0]["stream"].asString() == "/fw");

            /* each session's chosen C++ members (.xo-backlog/xo-websock/issues/13):
             * sender_ a ref to the sender printed in full above
             */
            for (Json::ArrayIndex k = 0; k < 2; ++k) {
                Json::Value const & mem = v[k]["_members_"];

                std::vector<std::string> names;
                for (Json::Value const & m : mem)
                    names.push_back(m["_name_"].asString());

                REQUIRE(names == std::vector<std::string>{"output_buf_", "sender_", "router_",
                                                          "outbound_q_"});
                REQUIRE(mem[1]["_metatype_"].asString() == "pointer");
                REQUIRE(mem[1]["_value_"]["_ref_"].asInt() == v[k]["sender"]["_id_"].asInt());
                REQUIRE(mem[2]["_canonical_type_"].asString() == "xo::web::WsSessionRouter");
                REQUIRE(mem[3]["_value_"].asString() == "0 queued");

                /* the router, nested: its own members.  sender_ the same
                 * sender (by most-derived address); subscription_v_ refs to
                 * exactly the subscriptions printed under the session
                 */
                Json::Value const & rmem = mem[2]["_value_"]["_members_"];

                std::vector<std::string> rnames;
                for (Json::Value const & m : rmem)
                    rnames.push_back(m["_name_"].asString());

                REQUIRE(rnames == std::vector<std::string>{"url_router_", "sender_", "pjson_",
                                                           "readjson_", "subscription_v_"});
                REQUIRE(rmem[0]["_metatype_"].asString() == "pointer");   /* a reference */
                /* ... to the server's url router, printed inside the server */
                REQUIRE(rmem[0]["_value_"]["_ref_"].asInt()
                        == root["_members_"][4]["_value_"]["_id_"].asInt());
                REQUIRE(rmem[1]["_value_"]["_ref_"].asInt() == v[k]["sender"]["_id_"].asInt());
                REQUIRE(rmem[3]["_value_"].asString() == "set");

                Json::Value const & slots = rmem[4]["_value_"];
                Json::Value const & subs = v[k]["subscriptions"];

                REQUIRE(rmem[4]["_metatype_"].asString() == "vector");
                REQUIRE(slots.size() == subs.size());
                for (Json::ArrayIndex i = 0; i < subs.size(); ++i)
                    REQUIRE(slots[i]["_ref_"].asInt() == subs[i]["_id_"].asInt());
            }

            /* the server's session table: session id -> a ref to that
             * session, printed in full in "sessions"
             */
            {
                Json::Value const * st = nullptr;
                for (Json::Value const & m : root["_members_"])
                    if (m["_name_"].asString() == "session_table_")
                        st = &m["_value_"];
                REQUIRE(st);

                Json::Value const & smap = (*st)["_members_"][1]["_value_"];
                REQUIRE(smap.size() == v.size());
                for (Json::ArrayIndex k = 0; k < v.size(); ++k) {
                    std::string const sid = std::to_string(v[k]["session_id"].asUInt64());
                    REQUIRE(smap[sid]["_ref_"].asInt() == v[k]["_id_"].asInt());
                }
                REQUIRE((*st)["_members_"][0]["_value_"].asUInt64()
                        > v[v.size() - 1]["session_id"].asUInt64());
            }

            /* nothing a printer opted in to is unprintable */
            {
                std::string const text = root.toStyledString();
                INFO(text);
                REQUIRE(text.find("\"_error_\"") == std::string::npos);
            }

            std::uint64_t second_id = v[1]["session_id"].asUInt64();

            /* the /fw endpoint is held by the router's map and by the one
             * subscription served: refcount 2
             */
            Json::Value const & eps = root["endpoints"];
            REQUIRE(eps.size() == 1);
            REQUIRE(eps[0]["pattern"].asString() == "/fw");
            REQUIRE(eps[0]["refcount"].asUInt() == 2);

            /* the edges, joined by id: the subscription's endpoint is THAT
             * endpoint; its sink's sender is ITS session's sender.  The sink
             * is held by the router's slot and by the endpoint's subscriber
             * (the test's SinkBox)
             */
            Json::Value const & sub = v[1]["subscriptions"][0];

            REQUIRE(sub["endpoint"]["_ref_"].asInt() == eps[0]["_id_"].asInt());
            REQUIRE(sub["sink"]["sender"]["_ref_"].asInt() == v[1]["sender"]["_id_"].asInt());
            REQUIRE(sub["sink"]["refcount"].asUInt() == 2);

            /* a closed session leaves the listing */
            REQUIRE(first->close(c_timeout));
            first.reset();

            REQUIRE(wait_until([&] { return n_session() == 1; }));
            REQUIRE(server_json()["sessions"][0]["session_id"].asUInt64() == second_id);
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WebserverLive.test.cpp */
