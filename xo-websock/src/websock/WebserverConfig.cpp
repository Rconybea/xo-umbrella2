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
            /* no members yet: a member is added as a printer opts in to
             * show it (.xo-backlog/xo-websock/issues/13)
             */
            StructReflector<WebserverConfig> sr;
        } /*reflect_self*/
    } /*namespace web*/
} /*namespace xo*/

/* end WebserverConfig.cpp */
