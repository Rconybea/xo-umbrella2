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
#include <algorithm>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <functional>
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

            /** the server's sessions, as @p root (a server snapshot) prints
             *  them: in full in session_table_'s session_map_, in session-id
             *  order -- no "sessions" list (xo-websock#15).  jsoncpp orders
             *  object keys as strings ("10" < "2"): sorted here by number
             **/
            Json::Value sessions_of(Json::Value const & root) {
                Json::Value smap;
                for (Json::Value const & m : root["_members_"])
                    if (m["_name_"].asString() == "session_table")
                        for (Json::Value const & tm : m["_value_"]["_members_"])
                            if (tm["_name_"].asString() == "session_map")
                                smap = tm["_value_"];

                std::vector<std::string> ids = smap.isObject() ? smap.getMemberNames()
                                                               : std::vector<std::string>();
                std::sort(ids.begin(), ids.end(), [](std::string const & x, std::string const & y) {
                    return std::stoull(x) < std::stoull(y);
                });

                Json::Value v(Json::arrayValue);
                for (std::string const & id : ids)
                    v.append(smap[id]);
                return v;
            }

            /** the server's endpoints, as @p srv (a server's json) prints
             *  them: in full in url_router_'s maps -- http_map_, then
             *  stream_map_, each in stem order -- at first encounter.  No
             *  "endpoints" list (xo-websock#15)
             **/
            Json::Value endpoints_of(Json::Value const & srv) {
                Json::Value ur;
                for (Json::Value const & m : srv["_members_"])
                    if (m["_name_"].asString() == "url_router")
                        ur = m["_value_"];

                Json::Value v(Json::arrayValue);
                for (char const * map : {"http_map", "stream_map"})
                    for (Json::Value const & m : ur["_members_"])
                        if (m["_name_"].asString() == map)
                            for (std::string const & stem : m["_value_"].getMemberNames())   /* jsoncpp: sorted */
                                v.append(m["_value_"][stem]);
                return v;
            }

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
            auto n_session = [&] { return sessions_of(server_json()).size(); };

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
            Json::Value const v = sessions_of(root);

            REQUIRE(!root.isMember("sessions"));   /* xo-websock#15 */

            INFO("json: " << root.toStyledString());

            /* a "_members_" entry by name: robust to member order */
            auto entry_of = [](Json::Value const & mem, std::string const & name) {
                for (Json::Value const & m : mem)
                    if (m["_name_"].asString() == name)
                        return m;
                return Json::Value();
            };
            /* session @p k's sender: in full in its reflected sender member
             * (shared, placed here -- xo-websock#16); and its id, the
             * sender's session_id
             */
            auto sender_of = [&](Json::ArrayIndex k) {
                return entry_of(v[k]["_members_"], "sender")["_value_"];
            };
            auto session_id_of = [&](Json::ArrayIndex k) {
                return entry_of(sender_of(k)["_members_"], "session_id")["_value_"].asUInt64();
            };

            /* by id, in connection order; distinct.  Printed from reflection
             * (xo-websock#16): _name_ is the short type, no view-model keys
             */
            REQUIRE(v.size() == 2);
            REQUIRE(v[0]["_name_"].asString() == "WebsocketSessionRecd");
            REQUIRE(v[0]["_canonical_type_"].asString() == "xo::web::WebsocketSessionRecd");
            REQUIRE(!v[0].isMember("session_id"));
            REQUIRE(!v[0].isMember("sender"));
            REQUIRE(session_id_of(0) < session_id_of(1));
            REQUIRE(v[0]["_id_"].asInt() != v[1]["_id_"].asInt());

            /* each session's sender, in full: open; its session's id */
            for (Json::ArrayIndex k = 0; k < 2; ++k) {
                Json::Value const sender = sender_of(k);   /* a copy: entry_of returns one */

                /* a template: its arguments follow */
                REQUIRE(sender["_name_"].asString().starts_with("WsSessionSender<"));
                REQUIRE(sender["_canonical_type_"].asString().starts_with("xo::web::WsSessionSender<"));
                REQUIRE(sender["_short_type_"].asString().starts_with("WsSessionSender<"));
                REQUIRE(sender.isMember("_id_"));
                /* no view-model keys (refcount, open, session_id): members */
                REQUIRE(!sender.isMember("refcount"));
                REQUIRE(!sender.isMember("open"));

                Json::Value const & smem = sender["_members_"];
                REQUIRE(smem[0]["_name_"].asString() == "session_id");
                REQUIRE(smem[0]["_value_"].asUInt64() > 0);
                /* a std::atomic<bool>: its load()ed value (xo-reflect#04) */
                REQUIRE(smem[1]["_name_"].asString() == "open");
                REQUIRE(smem[1]["_canonical_type_"].asString() == "std::atomic<bool>");
                REQUIRE(smem[1]["_value_"].asBool());
                /* target: the server, a raw pointer -- borrowed, a ref */
                REQUIRE(smem[2]["_name_"].asString() == "target");
                REQUIRE(smem[2]["_value_"]["_ref_"].asInt() == root["_id_"].asInt());
            }

            /* session @p k's subscriptions: in full in its router's
             * subscription_v_, slot order (xo-websock#15) -- no
             * "subscriptions" list
             */
            auto subs_of = [&](Json::ArrayIndex k) {
                Json::Value const router = entry_of(v[k]["_members_"], "router")["_value_"];
                return entry_of(router["_members_"], "subscription_v")["_value_"];
            };

            REQUIRE(!v[0].isMember("subscriptions"));
            REQUIRE(subs_of(0).empty());
            REQUIRE(subs_of(1).size() == 1);
            REQUIRE(entry_of(subs_of(1)[0]["_members_"], "stream_name")["_value_"].asString() == "/fw");

            for (Json::ArrayIndex k = 0; k < 2; ++k) {
                Json::Value const & mem = v[k]["_members_"];

                std::vector<std::string> names;
                for (Json::Value const & m : mem)
                    names.push_back(m["_name_"].asString());

                /* reflected, from the generic printer (xo-websock#16):
                 * unguarded first -- router, sender -- then output_buf,
                 * last_msg_seq and outbound_q under the session's mutex
                 * (xo-websock#15)
                 */
                REQUIRE(names == std::vector<std::string>{"router", "sender", "output_buf",
                                                          "last_msg_seq", "outbound_q"});
                /* output_buf placed here (owning): its OutputBuffer, or null
                 * between writes -- not a ref
                 */
                {
                    Json::Value const ob = entry_of(mem, "output_buf")["_value_"];   /* a copy: entry_of returns one */
                    REQUIRE((ob.isNull() || (ob.isObject() && ob.isMember("_id_"))));
                }
                /* sender: shared (rp), placed here in full -- not a ref */
                REQUIRE(entry_of(mem, "sender")["_metatype_"].asString() == "pointer");
                REQUIRE(entry_of(mem, "sender")["_value_"].isMember("_id_"));
                REQUIRE(entry_of(mem, "router")["_canonical_type_"].asString() == "xo::web::WsSessionRouter");
                /* the queued messages themselves (xo-reflect#04): none, settled */
                REQUIRE(entry_of(mem, "outbound_q")["_metatype_"].asString() == "vector");
                REQUIRE(entry_of(mem, "outbound_q")["_value_"].isArray());
                REQUIRE(entry_of(mem, "outbound_q")["_value_"].size() == 0);

                /* the router, nested: its own members.  sender_ the same
                 * sender (by most-derived address); subscription_v_ refs to
                 * exactly the subscriptions printed under the session
                 */
                Json::Value const router = entry_of(mem, "router");   /* a copy: entry_of returns one */
                Json::Value const & rmem = router["_value_"]["_members_"];

                std::vector<std::string> rnames;
                for (Json::Value const & m : rmem)
                    rnames.push_back(m["_name_"].asString());

                /* from reflection (generic printer, xo-websock#16): unguarded
                 * first, in reflection order -- readjson, url_router, sender,
                 * pjson -- then subscription_v under the router's mutex
                 * (xo-websock#15)
                 */
                REQUIRE(rnames == std::vector<std::string>{"readjson", "url_router", "sender",
                                                           "pjson", "subscription_v"});

                Json::Value const url_router = entry_of(rmem, "url_router");
                REQUIRE(url_router["_metatype_"].asString() == "pointer");   /* a raw pointer: borrowed */
                /* ... to the server's url router, printed inside the server */
                REQUIRE(url_router["_value_"]["_ref_"].asInt()
                        == entry_of(root["_members_"], "url_router")["_value_"]["_id_"].asInt());
                /* sender, pjson: borrowed -- refs (xo-websock#16) */
                REQUIRE(entry_of(rmem, "sender")["_value_"]["_ref_"].asInt() == sender_of(k)["_id_"].asInt());
                REQUIRE(entry_of(rmem, "pjson")["_value_"]["_ref_"].isInt());

                /* a unique_ptr to jsoncpp's reader, reflected with no members:
                 * present, so an empty object (xo-reflect#04)
                 */
                Json::Value const readjson = entry_of(rmem, "readjson");
                REQUIRE(readjson["_metatype_"].asString() == "pointer");
                REQUIRE(readjson["_value_"]["_name_"].asString() == "CharReader");
                REQUIRE(readjson["_value_"]["_members_"].empty());

                /* owning (unique_ptr): each subscription placed here, in
                 * full -- not a ref
                 */
                Json::Value const subscription_v = entry_of(rmem, "subscription_v");
                Json::Value const & slots = subscription_v["_value_"];

                REQUIRE(subscription_v["_metatype_"].asString() == "vector");
                for (Json::Value const & slot : slots) {
                    REQUIRE(slot.isMember("_id_"));
                    REQUIRE(slot["_name_"].asString() == "Subscription");
                }
            }

            /* the server's session table: session id -> that session, in
             * full (owning: unique_ptr), keyed by its id (xo-websock#15)
             */
            {
                Json::Value const * st = nullptr;
                for (Json::Value const & m : root["_members_"])
                    if (m["_name_"].asString() == "session_table")
                        st = &m["_value_"];
                REQUIRE(st);

                Json::Value const & smap = (*st)["_members_"][1]["_value_"];
                REQUIRE(smap.size() == v.size());
                for (Json::ArrayIndex k = 0; k < v.size(); ++k) {
                    std::string const sid = std::to_string(session_id_of(k));
                    REQUIRE(smap[sid].isMember("_id_"));
                    REQUIRE(smap[sid]["_id_"].asInt() == v[k]["_id_"].asInt());   /* keyed by its own id */
                }
                REQUIRE((*st)["_members_"][0]["_value_"].asUInt64()
                        > session_id_of(v.size() - 1));
            }

            /* nothing a printer opted in to is unprintable */
            {
                std::string const text = root.toStyledString();
                INFO(text);
                REQUIRE(text.find("\"_error_\"") == std::string::npos);
            }

            std::uint64_t second_id = session_id_of(1);

            /* the /fw endpoint is held by the router's map and by the one
             * subscription served: refcount 2
             */
            Json::Value const eps = endpoints_of(root);

            REQUIRE(!root.isMember("endpoints"));   /* xo-websock#15 */
            REQUIRE(eps.size() == 1);
            REQUIRE(eps[0]["pattern"].asString() == "/fw");
            REQUIRE(eps[0]["refcount"].asUInt() == 2);

            /* the edges, joined by id: the subscription's endpoint is THAT
             * endpoint; its sink's sender is ITS session's sender.  The sink
             * is held by the router's slot and by the endpoint's subscriber
             * (the test's SinkBox)
             */
            Json::Value const sub = subs_of(1)[0];

            /* from reflection (xo-websock#16): endpoint, sink are members.
             * The endpoint a ref -- the url router placed it first; the sink
             * in full, still by its own print_json (its keys)
             */
            REQUIRE(!sub.isMember("endpoint"));
            REQUIRE(!sub.isMember("sink"));
            REQUIRE(entry_of(sub["_members_"], "endpoint")["_value_"]["_ref_"].asInt() == eps[0]["_id_"].asInt());
            {
                Json::Value const sink = entry_of(sub["_members_"], "sink")["_value_"];
                REQUIRE(sink["_name_"].asString() == "WebsocketSink");
                REQUIRE(sink["sender"]["_ref_"].asInt() == sender_of(1)["_id_"].asInt());
                REQUIRE(sink["refcount"].asUInt() == 2);
            }

            /* a closed session leaves the listing */
            REQUIRE(first->close(c_timeout));
            first.reset();

            REQUIRE(wait_until([&] { return n_session() == 1; }));
            {
                Json::Value const left = sessions_of(server_json())[0];
                Json::Value const sender = entry_of(left["_members_"], "sender")["_value_"];
                REQUIRE(entry_of(sender["_members_"], "session_id")["_value_"].asUInt64() == second_id);
            }
        }

        namespace {
            /** values in a server snapshot that differ from run to run:
             *  replaced by "<redacted>" so the rest can be compared exactly.
             *  Top-level keys by name; _members_ entries by member name
             **/
            void redact(Json::Value & v) {
                static const std::vector<std::string> c_keys = {
                    "listen_port",
                    "_address_",       /* in "_unplaced_": an address */
                };
                static const std::vector<std::string> c_members = {
                    "listen_port",     /* the port the kernel chose (the server: generic printer, no "_") */
                    "port",            /* the config's copy: 0, or the port chosen */
                    "mount_origin",    /* a path */
                    "output_buf",      /* its id, or null between writes */
                };

                if (v.isArray()) {
                    for (Json::Value & x : v)
                        redact(x);
                } else if (v.isObject()) {
                    for (std::string const & k : c_keys)
                        if (v.isMember(k))
                            v[k] = "<redacted>";
                    if (v.isMember("_value_") && v.isMember("_name_")) {
                        std::string const name = v["_name_"].asString();
                        for (std::string const & m : c_members)
                            if (name == m)
                                v["_value_"] = "<redacted>";
                    }
                    for (std::string const & k : v.getMemberNames())
                        redact(v[k]);
                }
            }

            /** @p text, styled for a line-per-field diff **/
            std::string styled(Json::Value const & v) {
                Json::StreamWriterBuilder wb;
                wb["indentation"] = "  ";
                return Json::writeString(wb, v) + "\n";
            }

            /** the checked-in golden file @p name, under utest/golden/ **/
            std::string golden_path(std::string const & name) {
                return std::string(XO_WEBSOCK_UTEST_SOURCE_DIR) + "/golden/" + name;
            }

            /** the golden snapshot for the compiler this test is built with.
             *  A snapshot's type names (_canonical_type_, _short_type_) are
             *  the compiler's own spelling -- xo::reflect::type_name<T>,
             *  from __PRETTY_FUNCTION__ -- so each compiler has its own
             *  golden (.xo-backlog/xo-websock/issues/19).  nullptr: none yet
             **/
            constexpr char const * c_golden_name =
#if defined(__clang__)      /* before __GNUC__: clang defines both */
                "server-snapshot.clang.json";
#elif defined(__GNUC__)
                "server-snapshot.gcc.json";
#else
                nullptr;
#endif

            std::string read_file(std::string const & path) {
                std::ifstream in(path);
                std::stringstream ss;
                ss << in.rdbuf();
                return ss.str();
            }
        }

        TEST_CASE("live-server-snapshot-matches-golden", "[websock][live][json][golden]")
        {
            /* the whole server, as introspection prints it, against a
             * checked-in expectation: a printer change shows as a diff to
             * review (.xo-backlog/xo-websock/issues/14).  Every websock
             * printer appears: server, config, url router, endpoints (one
             * with a receiver), session table, sessions, senders, routers,
             * a subscription and its sink.
             *
             * One golden per compiler: type names are the compiler's
             * spelling (c_golden_name, .xo-backlog/xo-websock/issues/19).
             *
             * To bless a deliberate change:
             *   XO_UPDATE_GOLDEN=1 utest.websock.live "[golden]"
             * rewrites utest/golden/server-snapshot.<compiler>.json in the
             * source tree -- for the compiler this test was built with.
             * Bless under BOTH: gcc in an ordinary build; clang as CI builds
             * it, inside nix-shell ci-nxfs.nix -A xo-builder-shell
             * (.xo-backlog/ci/issues/03) with CC=clang CXX=clang++
             */
            auto box = std::make_shared<SinkBox>();

            LiveServer srv;
            srv.websrv_->register_stream_endpoint(box_descr("/fw", box));
            srv.websrv_->register_http_endpoint
                (HttpEndpointDescr("/status",
                                   [](HttpRequest const &) { return HttpResponse::json("{}"); }));

            std::int32_t port = srv.start();
            REQUIRE(port > 0);

            auto server_json = [&srv] {
                Webserver * server = srv.websrv_.get();
                std::stringstream ss;
                PrintJsonSingleton::instance()->print(server, &ss);
                return parse(ss.str());
            };

            /* two sessions, the second subscribed: wait on each step, so
             * the snapshot is of a settled state
             */
            WsTestClient first(port);
            REQUIRE(first.wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return sessions_of(server_json()).size() == 1; }));

            WsTestClient second(port);
            REQUIRE(second.wait_connected(c_timeout));
            REQUIRE(wait_until([&] { return sessions_of(server_json()).size() == 2; }));

            second.send(R"({"cmd": "subscribe", "stream": "/fw"})");
            REQUIRE(second.wait_received(1, c_timeout));
            REQUIRE(box->wait_sink(0));

            Json::Value snap = server_json();

            /* every ref's target placed (.xo-backlog/xo-printjson/issues/08):
             * no trailer
             */
            REQUIRE(!snap.isMember("_unplaced_"));

            redact(snap);
            std::string const actual = styled(snap);

            if (!c_golden_name)
                FAIL("no golden snapshot for this compiler: add a case to c_golden_name");

            std::string const path = golden_path(c_golden_name);

            if (char const * u = std::getenv("XO_UPDATE_GOLDEN"); u && *u && std::string(u) != "0") {
                std::ofstream(path) << actual;
                WARN("rewrote " << path);
                return;
            }

            {
                INFO("golden:  " << path << " -- missing: bless it with XO_UPDATE_GOLDEN=1, built with this compiler");
                REQUIRE(std::ifstream(path).good());
            }
            std::string const expected = read_file(path);

            if (actual != expected) {
                std::string const actual_path = "server-snapshot.actual.json";
                std::ofstream(actual_path) << actual;

                INFO("golden:  " << path);
                INFO("actual:  " << actual_path << " (in the test's working directory)");
                INFO("compare: diff -u <golden> <actual>; bless with XO_UPDATE_GOLDEN=1 (this compiler's golden only)");
                REQUIRE(actual == expected);
            }
        }
    } /*namespace ut*/
} /*namespace xo*/

/* end WebserverLive.test.cpp */
