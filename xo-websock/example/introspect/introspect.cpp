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
 *  Increment 1 of .xo-backlog/xo-websock/issues/10: the snapshot is just the
 *  server's port and run state.  Later increments add endpoints, sessions,
 *  subscriptions, and how they share objects.
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

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::StreamEndpointDescr;
    using xo::web::StreamReceiver;
    using xo::web::WebsocketSink;
    using xo::json::PrintJsonSingleton;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::fn::CallbackId;

    namespace web {
        /** what the page is told about this server.  Plain reflected value
         *  type: PrintJson renders it as a json object.
         *
         *  Increment 1: port and run state only.
         **/
        struct IntrospectSnapshot {
            static void reflect_self() {
                StructReflector<IntrospectSnapshot> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, listen_port);
                    REFLECT_MEMBER(sr, state);
                }
            }

            std::int32_t listen_port_ = 0;
            std::string state_;
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
