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
 *  Later: subscriptions, and how they share objects.
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
#include <xo/websock/WebsocketSink.hpp>
#include <xo/printjson/PrintJsonSingleton.hpp>
#include <xo/printjson/init_printjson.hpp>
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
    using xo::json::PrintJsonSingleton;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::fn::CallbackId;

    namespace web {
        /** one registered endpoint, for the page.  EndpointInfo with its
         *  kind as text (the enum is not reflected)
         **/
        struct IntrospectEndpoint {
            static void reflect_self() {
                StructReflector<IntrospectEndpoint> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, kind);
                    REFLECT_MEMBER(sr, stem);
                    REFLECT_MEMBER(sr, pattern);
                }
            }

            std::string kind_;
            std::string stem_;
            std::string pattern_;
        };

        /** one live websocket session, for the page **/
        struct IntrospectSession {
            static void reflect_self() {
                StructReflector<IntrospectSession> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, id);
                    REFLECT_MEMBER(sr, sender_open);
                    REFLECT_MEMBER(sr, n_subscription);
                }
            }

            std::uint64_t id_ = 0;
            bool sender_open_ = false;
            std::uint32_t n_subscription_ = 0;
        };

        /** what the page is told about this server.  Plain reflected value
         *  type: PrintJson renders it as a json object.
         **/
        struct IntrospectSnapshot {
            static void reflect_self() {
                IntrospectEndpoint::reflect_self();
                IntrospectSession::reflect_self();

                StructReflector<IntrospectSnapshot> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, listen_port);
                    REFLECT_MEMBER(sr, state);
                    REFLECT_MEMBER(sr, endpoints);
                    REFLECT_MEMBER(sr, sessions);
                }
            }

            std::int32_t listen_port_ = 0;
            std::string state_;
            /* http then stream, each by stem */
            std::vector<IntrospectEndpoint> endpoints_;
            /* by id */
            std::vector<IntrospectSession> sessions_;
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
                snap.listen_port_ = websrv_->listen_port();
                snap.state_ = RunstateUtil::runstate_descr(websrv_->state());

                for (EndpointInfo const & ep : websrv_->endpoints()) {
                    snap.endpoints_.push_back
                        (IntrospectEndpoint{endpoint_kind_descr(ep.kind_),
                                            ep.stem_,
                                            ep.uri_pattern_});
                }

                for (SessionInfo const & s : websrv_->sessions()) {
                    snap.sessions_.push_back
                        (IntrospectSession{s.session_id_,
                                           s.sender_open_,
                                           s.n_subscription_});
                }

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

    /* logging + reflection + json printing */
    InitSubsys<S_printjson_tag>::require();
    InitSubsys<S_indentlog2_tag>::require();
    Subsystem::initialize_all();

    AppConfig<S_indentlog2_tag> log_config{ Indentlog2Config(pp::PpConfig::plain(),
                                                             c_temp_arena_capacity) };
    AppContext<S_indentlog2_tag> log_cx{ log_config };

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
        = Webserver::make(WebserverConfig(port, false, false, false)
                              .with_mount_origin(origin.string()),
                          PrintJsonSingleton::instance());

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

    /* a stream nobody feeds yet; subscribing works, no frames arrive */
    websrv->register_stream_endpoint
        (StreamEndpointDescr("/demo/${id}",
                             [](rp<WebsocketSink> const &) { return CallbackId(1); },
                             [](CallbackId) {}));

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    websrv->start_webserver();

    /* wait until listening, to report the real port */
    while ((websrv->listen_port() == 0) && !s_stop)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cerr << "introspect: open http://localhost:" << websrv->listen_port()
              << "/  (Ctrl-C to stop)" << std::endl;

    while (!s_stop)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    websrv->stop_webserver();
    websrv->join_webserver();

    return 0;
}

/* end introspect.cpp */
