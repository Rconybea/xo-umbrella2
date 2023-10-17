/* @file Webserver.hpp */

#pragma once

#include "xo/refcnt/Displayable.hpp"
#include "xo/printjson/PrintJson.hpp"
#include "xo/webutil/HttpEndpointDescr.hpp"
#include "xo/webutil/StreamEndpointDescr.hpp"
#include <libwebsockets.h>  // temporary,  while moving callbacks
#include <thread>
#include <vector>
#include <memory>

namespace xo {
    namespace web {
        enum class Runstate { stopped, stop_requested, running };

        class RunstateUtil {
        public:
            static char const * runstate_descr(Runstate x);
        }; /*RunstateUtil*/

        inline std::ostream & operator<<(std::ostream &os, Runstate x) {
            os << RunstateUtil::runstate_descr(x);
            return os;
        } /*operator<<*/

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

        private:
            /* accept incoming http requests on this port# */
            std::int32_t port_ = 0;
            /* if true,  support https */
            bool tls_flag_ = false;
            /* see LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */
            bool host_check_flag_ = false;
            /* see lws_context_creation_info.retry_and_idle_policy */
            bool use_retry_flag_ = false;
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
         *      ^                      |
         *      |                      | .stop_webserver()
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
             */
            static ref::rp<Webserver> make(WebserverConfig const & ws_config,
                                           ref::rp<PrintJson> const & pjson);

            /* current state */
            virtual Runstate state() const = 0;
            virtual void register_http_endpoint(HttpEndpointDescr const & endpoint) = 0;
            virtual void register_stream_endpoint(StreamEndpointDescr const & endpoint) = 0;

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

            /* send text to a websocket session identified by session_id */
            virtual void send_text(uint32_t session_id,
                                   std::string text) = 0;

            // ----- Inherited from Displayable -----

            virtual void display(std::ostream & os) const;
        }; /*Webserver*/
    } /*namespace web*/
} /*namespace xo*/

/* end Webserver.hpp */
