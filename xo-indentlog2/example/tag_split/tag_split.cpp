/* example tag_split/tag_split.cpp
 *
 * @author Roland Conybeare, Jul 2026
 *
 * Demonstrate xo::pp::tag splitting when it runs out of soft right margin.
 *
 * A tag renders ":name value" on one line when it fits, and breaks to
 *   :name
 *     value
 * when the enclosing group exceeds the margin.  This needs the arena-backed
 * PrettySink (from xo-indentlog2): FlatSink never breaks (a split there just
 * emits its flat spaces), so tag-splitting can only be shown here.
 */

#include <xo/indentlog2/print/PrettySink.hpp>
#include <xo/arena/ArenaConfig.hpp>
#include <xo/ppsink/pretty.hpp>
#include <xo/ppsink/tag.hpp>
#include <cstdint>
#include <iostream>
#include <string>

using xo::pp::PrettySink;
using xo::pp::PpConfig;
using xo::pp::pretty;
using xo::pp::tag;
using xo::mm::ArenaConfig;

/* render tag("coordinate", 12345) through a PrettySink whose soft right margin
 * is @p margin (0 => the PpConfig default, i.e. effectively unbounded here).
 *
 * The tag's group width is  ":coordinate"(11) + 1 (fit-space) + "12345"(5) = 17,
 * so a margin below ~17 forces the split.
 */
static std::string
render_tag(std::uint32_t margin) {
    PrettySink pp(PpConfig::scratch_colored(margin ? margin : 135), nullptr /*out*/);
    pretty(pp, tag("coordinate", 12345));
    return std::string(pp.output());
}

int
main(int argc, char ** argv) {
    std::cout << "tag(\"coordinate\", 12345) at varying soft right margins:\n\n";

    for (std::uint32_t margin : { 30u, 12u }) {
        std::cout << "--- soft right margin = " << margin
                  << (margin >= 17 ? " (fits) ---\n" : " (splits) ---\n")
                  << render_tag(margin) << "\n\n";
    }

    return 0;
}

/* end tag_split.cpp */
