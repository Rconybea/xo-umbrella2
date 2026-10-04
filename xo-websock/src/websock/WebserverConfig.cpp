/** @file WebserverConfig.cpp
 *
 *  @author Roland Conybeare, Oct 2026
 *
 *  WebserverConfig is defined inline in Webserver.hpp; this is the home of
 *  its reflection.
 **/

#include "Webserver.hpp"
#include <xo/reflect/StructReflector.hpp>

namespace xo {
    using xo::reflect::StructReflector;

    namespace web {
        void
        WebserverConfig::reflect_self(reflect::TypeDescrTable * /*table*/)
        {
            StructReflector<WebserverConfig> sr;

            REFLECT_MEMBER(sr, port);
            REFLECT_MEMBER(sr, tls_flag);
            REFLECT_MEMBER(sr, host_check_flag);
            REFLECT_MEMBER(sr, use_retry_flag);
            REFLECT_MEMBER(sr, mount_origin);
        } /*reflect_self*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebserverConfig.cpp */
