/* file ObjectStatistics.cpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "ObjectStatistics.hpp"
#include <xo/reflect/TypeDescr.hpp>
#include <xo/ppsink/PrettyVector.hpp>   /* Prettifier<std::vector<T>> */
#include <xo/ppsink/pretty_struct.hpp>
#include <xo/ppsink/tag_ostream.hpp>    /* os << xtag(..) in display() */
#include <ostream>

namespace xo {
    namespace gc {
        /* NB in namespace xo::gc, not namespace xo: xo-reflect's headers no
         * longer bring legacy xo::xtag in, but xo-unit's quantity_iostream
         * (via GcStatistics.hpp) may.  One scope in, unqualified lookup stops
         * here rather than becoming ambiguous.  xrtag -> xtag: ppsink's xtag
         * does not escape, matching legacy tagstyle::raw.
         */
        using xo::pp::xtag;

        void
        PerObjectTypeStatistics::display(std::ostream & os) const
        {
            os << "<PerObjectTypeStatistics";
            if (td_)
                os << xtag("td", td_->short_name());
            else
                os << xtag("td", "nullptr");
            os << xtag("scanned_n", scanned_n_)
               << xtag("scanned_z", scanned_z_)
               << xtag("survive_n", survive_n_)
               << xtag("survive_z", survive_z_)
               << ">";
        }

        void
        ObjectStatistics::display(std::ostream & os) const
        {
            os << "<ObjectStatistics";

            std::size_t i = 0;
            for (const auto & x : per_type_stats_v_) {
                os << " :[" << i << "] " << x;
            }

            os << ">";
        }
    } /*namespace gc*/

} /*namespace xo*/

namespace xo::pp {
    void
    Prettifier<xo::gc::PerObjectTypeStatistics>::print(PpSink & sink,
                                                       const xo::gc::PerObjectTypeStatistics & x)
    {
        static constexpr std::string_view c_nullptr_str = "nullptr";

        /* NB legacy wrapped this in "if (x.td_) { .. } else { print nothing }",
         * which had two problems:
         *
         * 1. the nullptr case rendered as the empty string.  Harmless while
         *    tostr() went through display() (which does print ":td nullptr"),
         *    but ppsink's tostr() routes through this Prettifier -- so a
         *    default-constructed value stringified to "".
         * 2. inside `if (x.td_)` the ternary below could never take its else
         *    branch, so c_nullptr_str was dead.  It is clear from display()
         *    that ":td nullptr" was the intent.
         *
         * Print unconditionally, and let c_nullptr_str do its job.
         */

        /* bind to a local: field() captures by reference (like legacy refrtag),
         * and the conditional expression yields a prvalue.
         */
        const std::string_view name = (x.td_ ? x.td_->short_name() : c_nullptr_str);

        sink.pretty_struct("PerObjectTypeStatistics",
                           field("td", name),
                           field("scanned_n", x.scanned_n_),
                           field("scanned_z", x.scanned_z_),
                           field("survive_n", x.survive_n_),
                           field("survive_z", x.survive_z_));
    }

    void
    Prettifier<xo::gc::ObjectStatistics>::print(PpSink & sink,
                                                const xo::gc::ObjectStatistics & x)
    {
        sink.pretty_struct("ObjectTypeStatistics",
                           field("per_type_stats_v", x.per_type_stats_v_));
    }
} /*namespace xo::pp*/

/* end ObjectStatistics.cpp */
