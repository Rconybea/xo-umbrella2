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
 *    5a-5d. the snapshot is the server itself, printed by xo-websock's
 *       native json printers: each object once, others by ref, with ids and
 *       refcounts, so the page draws how the objects share each other
 *    5e. plus the application's own holds on sinks (the ticker's), which
 *       complete the refcounts -- reported as ids only, not drawn (the
 *       ticker box was dropped, .xo-backlog/xo-websock/issues/13)
 *
 *  The page files live in mount-origin/ beside the executable (cmake copies
 *  them there); found from the executable's own location, so this runs from
 *  any directory:
 *
 *    .build/xo-websock/example/introspect/websock_ex_introspect [port]
 *                                          [--type-maps=TEMPLATE]
 *                                          [--src-tree=ROOT]
 *                                          [--src-link=TEMPLATE]
 *
 *  then open http://localhost:<port>/ ; Ctrl-C to stop.
 *
 *  Type -> source locations (.xo-backlog/xo-websock/issues/12): built with
 *  -DXO_ENABLE_SOURCE_MAP=ON, cmake writes type-maps.json beside the
 *  executable -- where each subsystem's map is, for xo-websock and
 *  everything it depends on.  http://host:port/dyn/types merges them, on
 *  every request, so a rebuilt map shows without a restart.
 *  --type-maps=TEMPLATE overrides the locations: each subsystem's map is
 *  TEMPLATE with {subsystem} replaced by its name.
 *
 *  --src-tree=ROOT serves the source tree at ROOT (the repo root the maps'
 *  paths are relative to) as html pages, at
 *  http://host:port/dyn/src/<path>#L<line> -- the files of mapped types
 *  only.  Off unless given.
 *
 *  The page links each object to the source of its type (its
 *  _canonical_type_ looked up in /dyn/types), by the url template
 *  /dyn/types reports as "link":
 *  --src-link=TEMPLATE, with {file} and {line} replaced, e.g. a forgejo
 *  commit:
 *    --src-link=https://<host>/<owner>/xo-umbrella2/src/commit/<sha>/{file}#L{line}
 *  else, with --src-tree, /dyn/src/{file}#L{line}; else none -- no links.
 **/

#include <xo/websock/Webserver.hpp>
#include <xo/websock/cx/WebsockAppcx.hpp>
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/reflect/cx/ReflectAppcx.hpp>
#include <xo/websock/WebsocketSink.hpp>
#include <xo/printjson/JsonPrinter.hpp>
#include <xo/printjson/JsonMembers.hpp>   /* json::json_id */
#include <xo/ppsink/quoted_ostream.hpp>   /* quot(..) */
#include <xo/reflect/Reflect.hpp>
#include <xo/reflect/StructReflector.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/indentlog2/cx/Indentlog2Config.hpp>
#include <xo/indentlog2/init_indentlog2.hpp>
#include <xo/ppsink/PpStyle.hpp>
#include <xo/subsys/Subsystem.hpp>
#include <json/json.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace xo {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::web::StreamEndpointDescr;
    using xo::web::HttpEndpointDescr;
    using xo::web::HttpRequest;
    using xo::web::HttpResponse;
    using xo::web::html_escape;
    using xo::web::StreamReceiver;
    using xo::web::WebsocketSink;
    using xo::reflect::Reflect;
    using xo::reflect::StructReflector;
    using xo::reflect::TaggedPtr;
    using xo::json::PrintJson;
    using xo::json::JsonPrinter;
    using xo::pp::quot;
    using xo::fn::CallbackId;

    namespace web {
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

            /** call @p fn on each sink held, under the ticker's lock -- for
             *  introspection; @p fn must not send
             **/
            template <typename Fn>
            void visit_sinks(Fn && fn) const {
                std::lock_guard<std::mutex> lock(mutex_);

                for (auto const & ix : sink_map_)
                    fn(*(ix.second.get()));
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
            mutable std::mutex mutex_;
            std::uint32_t last_id_ = 0;
            std::map<std::uint32_t, rp<WebsocketSink>> sink_map_;
            /* ticker thread only */
            std::int64_t n_tick_ = 0;
        };

        /** what the page is told: the server itself; and, for refcount
         *  accounting only, the ids of the sinks the application holds (the
         *  ticker's) -- holds the server's json cannot show.  PrintJson
         *  follows the server pointer to its printer (installed by
         *  WebsockAppcx)
         **/
        struct IntrospectSnapshot {
            static void reflect_self() {
                StructReflector<IntrospectSnapshot> sr;

                if (sr.is_incomplete()) {
                    REFLECT_MEMBER(sr, server);
                    REFLECT_MEMBER(sr, app_holds);
                }
            }

            Webserver * server_ = nullptr;
            /** one id per hold, the same string as the sink's own "id" **/
            std::vector<std::string> app_holds_;
        };

        /** answers {"cmd": "send", "msg": "refresh"} with a snapshot, on the
         *  asking subscription
         **/
        class IntrospectReceiver : public StreamReceiver {
        public:
            IntrospectReceiver(Webserver * websrv, Ticker * ticker)
                : websrv_{websrv}, ticker_{ticker} {}

            void receive(rp<WebsocketSink> const & sink, Json::Value const & msg) override {
                if (!msg.isString() || msg.asString() != "refresh")
                    throw std::runtime_error("expected \"refresh\"");

                IntrospectSnapshot snap;
                snap.server_ = websrv_;
                /* by most-derived address: the id the sink's printer writes */
                ticker_->visit_sinks([&snap](WebsocketSink const & s) {
                        snap.app_holds_.push_back(xo::json::json_id(dynamic_cast<void const *>(&s)));
                    });

                sink->notify_ev_tp(Reflect::make_tp(&snap));
            }

        private:
            /* borrowed: both outlive the server's use of this receiver */
            Webserver * websrv_ = nullptr;
            Ticker * ticker_ = nullptr;
        };

        /** where each subsystem's type -> source map is (xo-type-src-map,
         *  one per subsystem); merged on request, so never stale
         **/
        class TypeMaps {
        public:
            /** from @p list_file (cmake's type-maps.json).  A non-empty
             *  @p tmpl overrides each map's location: @p tmpl with
             *  "{subsystem}" replaced by the subsystem's name
             **/
            static TypeMaps from_list(std::filesystem::path const & list_file,
                                      std::string const & tmpl) {
                TypeMaps retval;

                std::ifstream in(list_file);
                if (!in)
                    return retval;

                Json::Value list;
                Json::CharReaderBuilder rb;
                std::string errs;
                if (!Json::parseFromStream(rb, in, &list, &errs))
                    throw std::runtime_error("TypeMaps: cannot parse "
                                             + list_file.string() + ": " + errs);

                Json::Value const & maps = list["maps"];
                for (auto const & name : maps.getMemberNames()) {
                    std::string path = maps[name].asString();

                    if (!tmpl.empty()) {
                        path = tmpl;
                        std::string::size_type p = path.find("{subsystem}");
                        if (p != std::string::npos)
                            path.replace(p, std::string("{subsystem}").size(), name);
                    }

                    retval.maps_.emplace_back(name, path);
                }

                return retval;
            }

            bool empty() const { return maps_.empty(); }

            /** true iff @p rel (repo-relative) is the file of some type in
             *  the merged maps -- read now, like merged()
             **/
            bool names_file(std::string const & rel) const {
                Json::Value const m = this->merged();
                Json::Value const & types = m["types"];

                for (auto const & tname : types.getMemberNames()) {
                    if (types[tname]["file"].asString() == rel)
                        return true;
                }

                return false;
            }

            /** the union of the maps, as json -- read from the files now, so
             *  a rebuilt map shows without a restart.  A missing or
             *  unreadable map is listed under "missing"; a name in two maps
             *  at different places is left out of "types" and listed under
             *  "conflicts" -- as xo-type-src-merge does
             **/
            Json::Value merged() const {
                Json::Value types(Json::objectValue);
                Json::Value owner(Json::objectValue);    /* name -> subsystem */
                Json::Value conflicts(Json::objectValue);
                Json::Value used(Json::arrayValue);
                Json::Value missing(Json::arrayValue);

                for (auto const & [name, path] : maps_) {
                    std::ifstream in(path);
                    Json::Value m;
                    Json::CharReaderBuilder rb;
                    std::string errs;

                    if (!in || !Json::parseFromStream(rb, in, &m, &errs)) {
                        missing.append(name);
                        continue;
                    }

                    used.append(name);

                    Json::Value const & mtypes = m["types"];
                    for (auto const & tname : mtypes.getMemberNames()) {
                        Json::Value const & loc = mtypes[tname];

                        if (!types.isMember(tname)) {
                            types[tname] = loc;
                            owner[tname] = name;
                        } else if (types[tname] != loc) {
                            if (!conflicts.isMember(tname))
                                conflicts[tname].append(located(owner[tname].asString(),
                                                                types[tname]));
                            conflicts[tname].append(located(name, loc));
                        }
                    }
                }

                for (auto const & tname : conflicts.getMemberNames())
                    types.removeMember(tname);

                Json::Value out(Json::objectValue);
                out["format"] = "xo-type-src-map/1";
                out["subsystems"] = used;
                out["missing"] = missing;
                out["types"] = types;
                out["conflicts"] = conflicts;

                return out;
            }

        private:
            static Json::Value located(std::string const & subsystem, Json::Value const & loc) {
                Json::Value x = loc;
                x["subsystem"] = subsystem;
                return x;
            }

            /* (subsystem, path to its types.json) */
            std::vector<std::pair<std::string, std::string>> maps_;
        };

        /** @brief a source tree, served read-only as html pages with
         *  numbered lines -- the "uncommitted tree" link provider
         *  (.xo-backlog/xo-websock/issues/12, step 4b).
         *
         *  Confined: a path must be relative, without "..", and resolve --
         *  symlinks followed -- to a regular file inside the root; and it
         *  must be the file of some type in the maps, so this is no general
         *  file server.
         **/
        class SourceTree {
        public:
            /** @p root must exist **/
            explicit SourceTree(std::filesystem::path const & root)
                : root_{std::filesystem::canonical(root)} {}

            std::filesystem::path const & root() const { return root_; }

            /** the page for repo-relative @p rel, or why not **/
            HttpResponse serve(std::string const & rel, TypeMaps const & maps) const {
                namespace fs = std::filesystem;

                if (rel.empty())
                    return HttpResponse::not_found("no file named");
                if (rel.front() == '/')
                    return HttpResponse::forbidden("absolute path [" + rel + "]");

                /* no ".." (nor "." or empty) segment */
                for (std::size_t b = 0; b <= rel.size(); ) {
                    std::size_t e = rel.find('/', b);
                    if (e == std::string::npos)
                        e = rel.size();

                    std::string_view seg(rel.data() + b, e - b);
                    if (seg.empty() || seg == "." || seg == "..")
                        return HttpResponse::forbidden("path [" + rel + "] is not plain");

                    b = e + 1;
                }

                std::error_code ec;
                fs::path file = fs::canonical(this->root_ / rel, ec);

                if (ec)
                    return HttpResponse::not_found("no file [" + rel + "]");

                /* inside the root, after symlinks */
                auto [ri, fi] = std::mismatch(root_.begin(), root_.end(), file.begin(), file.end());
                if (ri != root_.end())
                    return HttpResponse::forbidden("[" + rel + "] is outside the source tree");

                if (!fs::is_regular_file(file))
                    return HttpResponse::not_found("[" + rel + "] is not a file");

                if (!maps.names_file(rel))
                    return HttpResponse::not_found("[" + rel + "] is not a file of any mapped type");

                std::ifstream in(file);
                if (!in)
                    return HttpResponse::not_found("cannot read [" + rel + "]");

                return HttpResponse::html(this->page(rel, in));
            }

        private:
            /** @p in as a page: a line per <span id="L<n>">, the target
             *  highlighted
             **/
            std::string page(std::string const & rel, std::istream & in) const {
                std::string out;

                out += "<!doctype html><html><head><meta charset=\"utf-8\"><title>"
                    + html_escape(rel) + "</title><style>"
                    "body{margin:0;font:13px/1.45 ui-monospace,SFMono-Regular,Menlo,monospace}"
                    "header{position:sticky;top:0;background:#f4f4f4;padding:6px 12px;"
                    "border-bottom:1px solid #ccc;font-family:sans-serif}"
                    ".note{color:#666;font-size:12px}"
                    "pre{margin:0;padding:4px 0}"
                    ".l{display:block;scroll-margin-top:5em}"
                    ".l:target{background:#fff3b0}"
                    ".n{display:inline-block;width:6ch;padding-right:1.5ch;text-align:right;"
                    "color:#999;text-decoration:none;user-select:none}"
                    "</style></head><body><header><b>" + html_escape(rel) + "</b>"
                    "<div class=\"note\">the working tree at " + html_escape(root_.string())
                    + ": if files changed since this program was built, lines may have moved"
                    "</div></header><pre>";

                std::string line;
                for (int n = 1; std::getline(in, line); ++n) {
                    std::string id = "L" + std::to_string(n);

                    out += "<span class=\"l\" id=\"" + id + "\"><a class=\"n\" href=\"#"
                        + id + "\">" + std::to_string(n) + "</a>" + html_escape(line)
                        + "</span>";
                }

                out += "</pre></body></html>";

                return out;
            }

        private:
            /** canonical **/
            std::filesystem::path root_;
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

    using xo::web::TypeMaps;
    using xo::web::SourceTree;

    std::int32_t port = 7681;
    std::string type_maps_tmpl;
    std::string src_tree;
    std::string src_link;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.rfind("--type-maps=", 0) == 0)
            type_maps_tmpl = arg.substr(std::string("--type-maps=").size());
        else if (arg.rfind("--src-tree=", 0) == 0)
            src_tree = arg.substr(std::string("--src-tree=").size());
        else if (arg.rfind("--src-link=", 0) == 0)
            src_link = arg.substr(std::string("--src-link=").size());
        else
            port = std::atoi(argv[i]);
    }

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

    /* the ticker: each /demo subscriber gets a counter once a second.  Made
     * before /introspect, whose receiver reports its holds
     */
    auto ticker = std::make_shared<xo::web::Ticker>();

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
                             new IntrospectReceiver(websrv.get(), ticker.get())));

    /* demo endpoints: something besides /introspect to look at.
     * http endpoints are served under the server's dynamic mount, /dyn --
     * so this one answers http://host:port/dyn/hello/<name>
     */
    websrv->register_http_endpoint
        (HttpEndpointDescr("/hello/${name}",
                           [](HttpRequest const & req)
                               {
                                   return HttpResponse::html("<html>hello, "
                                                             + html_escape(req.var("name"))
                                                             + "</html>");
                               }));

    /* type -> source locations: http://host:port/dyn/types */
    auto type_maps = std::make_shared<TypeMaps>
        (TypeMaps::from_list(exe_dir(argv[0]) / "type-maps.json", type_maps_tmpl));

    if (type_maps->empty())
        std::cerr << "introspect: no type maps (build with -DXO_ENABLE_SOURCE_MAP=ON):"
                     " /dyn/types is empty" << std::endl;

    /* how the page links a type to its source: see the file comment */
    if (src_link.empty() && !src_tree.empty())
        src_link = "/dyn/src/{file}#L{line}";

    websrv->register_http_endpoint
        (HttpEndpointDescr("/types",
                           [type_maps, src_link](HttpRequest const &)
                               {
                                   Json::Value out = type_maps->merged();
                                   out["link"] = (src_link.empty()
                                                  ? Json::Value(Json::nullValue)
                                                  : Json::Value(src_link));

                                   Json::StreamWriterBuilder wb;
                                   wb["indentation"] = "";

                                   return HttpResponse::json(Json::writeString(wb, out));
                               }));

    /* the source tree: http://host:port/dyn/src/<path>, when asked for */
    if (!src_tree.empty()) {
        std::error_code ec;
        if (!std::filesystem::is_directory(src_tree, ec)) {
            std::cerr << "introspect: --src-tree: not a directory: " << src_tree << std::endl;
            return 1;
        }

        auto tree = std::make_shared<SourceTree>(src_tree);

        std::cerr << "introspect: serving source tree " << tree->root()
                  << " at /dyn/src/" << std::endl;

        websrv->register_http_endpoint
            (HttpEndpointDescr("/src/${path...}",
                               [tree, type_maps](HttpRequest const & req)
                                   {
                                       return tree->serve(std::string(req.var("path")),
                                                          *type_maps);
                                   }));
    }

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
