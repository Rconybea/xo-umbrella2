/** @file SetupObject2.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace json { class PrintJson; }

    namespace scm {
        struct SetupObject2 {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            static bool register_facets();
            static bool register_types(obj<ACollector> gc);

            /** describe object2's D-types to xo-reflect. **/
            static void reflect_types();

            /** install object2's json printers into @p p_pjson.
             *
             *  Lives here, rather than in xo-printjson, because printjson must
             *  not know about the object model.  It works only because
             *  xo-printjson is levelled BELOW xo-object2 -- see the
             *  subsystem-list.  Same shape as
             *  xo::eigen::EigenUtil::provide_json_printers.
             **/
            static void provide_json_printers(xo::json::PrintJson * p_pjson);
        };
    }
}

/* end object2_register_facets.hpp */
