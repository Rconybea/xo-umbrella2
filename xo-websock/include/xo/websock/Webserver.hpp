/* @file Webserver.hpp */

#pragma once

#include "EndpointKind.hpp"
#include <xo/printjson/PrintJson.hpp>
#include <xo/webutil/HttpEndpointDescr.hpp>
#include <xo/webutil/StreamEndpointDescr.hpp>
#include <xo/refcnt/Displayable.hpp>
#include <xo/ppsink/Prettifier.hpp>   /* Prettifier<>, XO_PRETTIFIER_DECLARE */
#include <libwebsockets.h> // temporary,  while moving callbacks
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace xo {
    class WebsockAppcx;

    namespace web {
        enum class Runstate { stopped, stop_requested, running };

        class RunstateUtil {
        public:
            static char const * runstate_descr(Runstate x);
        }; /*RunstateUtil*/

        class WebserverConfig {
        public:
            WebserverConfig() = default;
            WebserverConfig(std::int32_t port,
                            bool tls_flag,
                            bool host_check_flag,
                            bool use_retry_flag)
                : port_{port},
                  tls_flag_{tls_flag},
                  host_check_flag_{host_check_flag},
                  use_retry_flag_{use_retry_flag} {}

            std::int32_t port() const { return port_; }
            bool tls_flag() const { return tls_flag_; }
            bool host_check_flag() const { return host_check_flag_; }
            bool use_retry_flag() const { return use_retry_flag_; }
            std::string const & mount_origin() const { return mount_origin_; }

            /** copy of this config serving static files from @p dir.
             *  Relative paths resolve against the process's working
             *  directory, when the server starts.
             **/
            WebserverConfig with_mount_origin(std::string dir) const {
                WebserverConfig retval = *this;
                retval.mount_origin_ = std::move(dir);
                return retval;
            }

        private:
            /* accept incoming http requests on this port# */
            std::int32_t port_ = 0;
            /* if true,  support https */
            bool tls_flag_ = false;
            /* see LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */
            bool host_check_flag_ = false;
            /* see lws_context_creation_info.retry_and_idle_policy */
            bool use_retry_flag_ = false;
            /* directory served at "/" (index.html by default).  A request
             * naming no file there falls through to the dynamic-content
             * handler, whose "no dynamic content" page is the symptom of a
             * wrong directory
             */
            std::string mount_origin_ = "./mount-origin";
        }; /*WebserverConfig*/

        /* libwebsocket:
         * 1. doesn't support multiple threads
         *    (actually, looks like it does on further examination)
         * 2. doesn't expose listening ports etc (at least afaik);
         *    in other words it expects to take over application's main thread
         *
         * enforce this property by making webserver a singleton
         *
         *    .state      .start_webserver()    .state
         *   +---------+  -------------------> +---------+
         *   | stopped |                       | running |
         *   +---------+                       +---------+
         *      ^                                  |
         *      |                                  | .stop_webserver()
         *      |                                  |
         *   +----------------+                    |
         *   | stop_requested | <------------------/
         *   +----------------+
         *
         */
        class Webserver : public ref::Displayable {
        public:
            using Alist = xo::web::Alist;
            using PrintJson = xo::json::PrintJson;

        public:
            /* note: although webserver allows creating multiple instances,
             *       the underlying libwebsocket library is not advertised to be
             *       threadsafe
             *
             * Made from @p cx: the server prints with cx's PrintJson, into
             * which cx installed xo-websock's json printers -- so no server
             * exists without them (.xo-backlog/xo-websock/issues/11)
             */
            static rp<Webserver> make(WebsockAppcx const & cx,
                                      WebserverConfig const & ws_config);

            /* current state */
            virtual Runstate state() const = 0;
            /* port this server is accepting connections on; 0 until it is
             * listening (start_webserver() returns before that), and again
             * once stopped.  With WebserverConfig port 0 the OS picks the
             * port, and this is how to learn it.
             */
            virtual std::int32_t listen_port() const = 0;
            /* register_*_endpoint: throws std::runtime_error if an endpoint of
             * the same kind with the same stem is already registered; see
             * UrlRouter
             */
            virtual void register_http_endpoint(HttpEndpointDescr const & endpoint) = 0;
            virtual void register_stream_endpoint(StreamEndpointDescr const & endpoint) = 0;

            /* unregister_*_endpoint: remove the endpoint registered with
             * exactly @p uri_pattern; false if there is none.
             *
             * Takes effect for new requests at once.  Removing a stream
             * endpoint also ends its live subscriptions, in every session:
             * shortly after, on the webserver's thread, each runs the
             * endpoint's unsubscribe and its client gets
             *   {"cmd": "unsubscribed", "sub_id": N, "reason": "endpoint removed"}
             * Callable from any thread, before or while the server runs.
             * See .xo-backlog/xo-websock/issues/07.
             */
            virtual bool unregister_http_endpoint(std::string const & uri_pattern) = 0;
            virtual bool unregister_stream_endpoint(std::string const & uri_pattern) = 0;

            /* call fn on every registered endpoint (http, then stream, each by
             * stem), for introspection.  Any thread.  fn runs under the
             * router's lock: it must not register or unregister endpoints
             */
            virtual void visit_endpoints(EndpointVisitor const & fn) const = 0;

            /* call fn on every live websocket session, in id order, for
             * introspection.  Any thread.  The session is private to the
             * server, so fn gets it as a TaggedPtr -- for PrintJson, whose
             * printer for it the websock context installed.  fn runs under
             * the session table's lock: it must not send, or open/close
             * sessions
             */
            using SessionVisitor = std::function<void (xo::reflect::TaggedPtr session)>;
            virtual void visit_sessions(SessionVisitor const & fn) const = 0;

            /* start thread for this webserver; idempotent */
            virtual void start_webserver() = 0;
            /* stop thread for this webserver;  suitable for calling
             * from interrupt handler
             */
            virtual void interrupt_stop_webserver() = 0;
            /* stop thread for this webserver; idempotent */
            virtual void stop_webserver() = 0;
            /* wait until webserver thread stopped */
            virtual void join_webserver() = 0;

            /* send text to a websocket session identified by session_id.
             * Dropped if that session has closed; ids are never reused
             */
            virtual void send_text(uint64_t session_id,
                                   std::string text) = 0;

            // ----- Inherited from Displayable -----

            virtual void pretty(PpSink & pp) const override;
            virtual std::string display_string() const override;
        }; /*Webserver*/
    } /*namespace web*/

    namespace pp {
        XO_PRETTIFIER_DECLARE(xo::web::Runstate);
    }
} /*namespace xo*/

/* end Webserver.hpp */
