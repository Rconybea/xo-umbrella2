/** @file introspect.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 *
 *  A webserver that shows its own state in the browser.
 *
 *  Serves ./mount-origin (index.html + introspect.js) and a websocket stream,
 *  "/introspect".  The page subscribes, and on {"cmd": "send", "msg":
 *  "refresh"} this program replies with a snapshot of its own state, which
 *  the page draws with d3.
 *
 *  Increments of .xo-backlog/xo-websock/issues/10:
 *    1. the server's port and run state
 *    2. its registered endpoints -- plus two demo endpoints, so there is more
 *       than /introspect to see
 *    3. its live websocket sessions: one per connected page
 *    4. each session's subscriptions -- /demo/${id} is now a ticker, so a
 *       subscription to it has traffic
 *    5a. the snapshot is the server itself, printed by xo-websock's
 *       Webserver json printer rather than assembled here
 *  Later: how they share objects.
 *
 *  The page files live in mount-origin/ beside the executable (cmake copies
 *  them there); found from the executable's own location, so this runs from
 *  any directory:
 *
 *    .build/xo-websock/example/introspect/websock_ex_introspect [port]
 *
 *  then open http://localhost:<port>/ ; Ctrl-C to stop.
 **/

#include <xo/websock/Webserver.hpp>
#include <xo/websock/cx/WebsockAppcx.hpp>
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/reflect/cx/ReflectAppcx.hpp>
#include <xo/websock/WebsocketSink.hpp>
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/subsys/Subsystem.hpp>
#include <json/json.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::StreamEndpointDescr;
    using xo::web::HttpEndpointDescr;
    using xo::web::Alist;
    using xo::web::StreamReceiver;
    using xo::web::WebsocketSink;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::fn::CallbackId;

    namespace web {
        /** what the page is told: the server itself.  PrintJson follows the
         *  pointer to the Webserver json printer (xo/websock/websock_json.hpp),
         *  which WebsockAppcx installed.  A struct, so later increments can
         *  add the application's own objects beside it
         **/
        struct IntrospectSnapshot {
            static void reflect_self() {
                StructReflector<IntrospectSnapshot> sr;

                if (sr.is_incomplete())
                    REFLECT_MEMBER(sr, server);
            }

            Webserver * server_ = nullptr;
        };

        /** /demo/${id}: sends each subscriber a counter, once a second.
         *  Keeps its sinks by callback id, for unsubscribe.
         **/
        class Ticker {
        public:
            CallbackId subscribe(rp<WebsocketSink> const & sink) {
                std::lock_guard<std::mutex> lock(mutex_);

                std::uint32_t id = ++last_id_;
                sink_map_[id] = sink;

                return CallbackId(id);
            }

            void unsubscribe(CallbackId id) {
                std::lock_guard<std::mutex> lock(mutex_);

                sink_map_.erase(id.id());
            }

            /* one tick to every subscriber.  Sends with the lock RELEASED:
             * a send enters the server, which may be running subscribe on
             * its own thread, waiting for this lock
             */
            void tick() {
                std::vector<rp<WebsocketSink>> sink_v;

                {
                    std::lock_guard<std::mutex> lock(mutex_);

                    for (auto const & ix : sink_map_)
                        sink_v.push_back(ix.second);
                }

                std::int64_t n = ++n_tick_;

                for (auto const & sink : sink_v)
                    sink->notify_ev_tp(Reflect::make_tp(&n));
            }

        private:
            std::mutex mutex_;
            std::uint32_t last_id_ = 0;
            std::map<std::uint32_t, rp<WebsocketSink>> sink_map_;
            /* ticker thread only */
            std::int64_t n_tick_ = 0;
        };

        /** answers {"cmd": "send", "msg": "refresh"} with a snapshot, on the
         *  asking subscription
         **/
        class IntrospectReceiver : public StreamReceiver {
        public:
            explicit IntrospectReceiver(Webserver * websrv) : websrv_{websrv} {}

            void receive(rp<WebsocketSink> const & sink, Json::Value const & msg) override {
                if (!msg.isString() || msg.asString() != "refresh")
                    throw std::runtime_error("expected \"refresh\"");

                IntrospectSnapshot snap;
                snap.server_ = websrv_;

                sink->notify_ev_tp(Reflect::make_tp(&snap));
            }

        private:
            /* borrowed: the server outlives its endpoints' use here */
            Webserver * websrv_ = nullptr;
        };
    } /*namespace web*/
} /*namespace xo*/

namespace {
    std::atomic<bool> s_stop{false};

    /** directory holding this executable.  /proc/self/exe on linux; else
     *  argv[0], which holds a path whenever the program was run by path --
     *  examples are not installed on PATH
     **/
    std::filesystem::path exe_dir(char const * argv0) {
        namespace fs = std::filesystem;

        std::error_code ec;
        fs::path self = fs::read_symlink("/proc/self/exe", ec);

        if (ec)
            self = fs::absolute(argv0, ec);

        return self.parent_path();
    }

    extern "C" void on_signal(int) { s_stop = true; }

    /** capacity for the thread-local scratch arena behind tostr() **/
    constexpr std::uint32_t c_temp_arena_capacity = 64 * 1024;
}

int
main(int argc, char * argv[])
{
    using namespace xo;
    using xo::web::IntrospectSnapshot;
    using xo::web::IntrospectReceiver;

    std::int32_t port = (argc > 1) ? std::atoi(argv[1]) : 7681;

    /* the subsystem stack: logging, reflection, json printing, websock.
     * Establishing the websock context installs its json printers; a
     * Webserver is made from it
     */
    using IntrospectConfig = AppConfig<S_indentlog2_tag, S_reflect_tag,
                                       S_printjson_tag, S_websock_tag>;
    using IntrospectContext = AppContext<S_indentlog2_tag, S_reflect_tag,
                                         S_printjson_tag, S_websock_tag>;

    IntrospectConfig app_config{ Indentlog2Config(pp::PpConfig::plain(),
                                                  c_temp_arena_capacity),
                                 ReflectConfig(),
                                 PrintJsonConfig(),
                                 WebsockConfig() };
    IntrospectContext app_cx{ app_config };

    Subsystem::initialize_all();

    IntrospectSnapshot::reflect_self();

    /* the page: served from beside the executable, not the cwd.  Checked up
     * front -- otherwise every request falls through to the server's "no
     * dynamic content" page, which does not say why
     */
    std::filesystem::path origin = exe_dir(argv[0]) / "mount-origin";

    if (!std::filesystem::exists(origin / "index.html")) {
        std::cerr << "introspect: page not found: " << (origin / "index.html")
                  << "\n  (cmake copies mount-origin/ beside the executable;"
                     " rebuild target websock_ex_introspect)" << std::endl;
        return 1;
    }

    rp<Webserver> websrv
        = Webserver::make(app_cx.cx<S_websock_tag>(),
                          WebserverConfig(port, false, false, false)
                              .with_mount_origin(origin.string()));

    websrv->register_stream_endpoint
        (StreamEndpointDescr("/introspect",
                             /* nothing to attach: every frame is a reply */
                             [](rp<WebsocketSink> const &) { return CallbackId(1); },
                             [](CallbackId) {},
                             new IntrospectReceiver(websrv.get())));

    /* demo endpoints: something besides /introspect to look at.
     * http endpoints are served under the server's dynamic mount, /dyn --
     * so this one answers http://host:port/dyn/hello/<name>
     */
    websrv->register_http_endpoint
        (HttpEndpointDescr("/hello/${name}",
                           [](std::string const &, Alist const & args, std::ostream * p_os)
                               {
                                   *p_os << "<html>hello, " << args.lookup("name") << "</html>";
                               }));

    /* a ticker: each subscriber gets a counter once a second */
    auto ticker = std::make_shared<xo::web::Ticker>();

    websrv->register_stream_endpoint
        (StreamEndpointDescr("/demo/${id}",
                             [ticker](rp<WebsocketSink> const & sink) { return ticker->subscribe(sink); },
                             [ticker](CallbackId id) { ticker->unsubscribe(id); }));

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    websrv->start_webserver();

    /* wait until listening, to report the real port */
    while ((websrv->listen_port() == 0) && !s_stop)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cerr << "introspect: open http://localhost:" << websrv->listen_port()
              << "/  (Ctrl-C to stop)" << std::endl;

    /* main thread drives the ticker; 100ms steps so Ctrl-C is prompt */
    for (int i = 1; !s_stop; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (i % 10 == 0)
            ticker->tick();
    }

    websrv->stop_webserver();
    websrv->join_webserver();

    return 0;
}

/* end introspect.cpp */
