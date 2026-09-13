/** @file SetupStringtable2.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include <xo/alloc2/Collector.hpp>

namespace xo {
    namespace json { class PrintJson; }

    namespace scm {
        struct SetupStringtable2 {
        public:
            using ACollector = xo::mm::ACollector;

        public:
            /** Register object2 (facet,impl) combinations with FacetRegistry **/
            static bool register_facets();
            /** Register types with garbage collector **/
            static bool register_types(obj<ACollector> gc);

            /** install stringtable2's json printers into @p p_pjson.
             *
             *  Lives here rather than in xo-printjson, for the reason
             *  SetupObject2::provide_json_printers gives: printjson must not
             *  know about the object model.
             *
             *  Not optional the way DFloat's printer was.  DString does not
             *  reflect member-wise -- see DString::self_tp -- so without this
             *  a DString renders as an opaque atom and its characters go
             *  nowhere.
             **/
            static void provide_json_printers(xo::json::PrintJson * p_pjson);
        };
    }
}

/* end SetupStringtable2.hpp */
