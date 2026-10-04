/* @file WebserverConfig.hpp */

#pragma once

#include <xo/reflect/TypeDescr.hpp>
#include <cstdint>

namespace xo {
    namespace web {

        class WebserverConfig {
        public:
            /** describe WebserverConfig to xo-reflect.
             *  Called once, by websock_reflect_types() (websock_reflect.hpp).
             *  @p table is not used yet: xo-reflect's registration uses its
             *  process-wide table
             **/
            static void reflect_self(reflect::TypeDescrTable * table);

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
            /* reads private members, for "_members_" (websock_json.cpp) */
            friend class JsonPrinter_WebserverConfig;

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

    } /*namespace web*/
} /*namespace xo*/

/* end WebserverConfig.hpp */
