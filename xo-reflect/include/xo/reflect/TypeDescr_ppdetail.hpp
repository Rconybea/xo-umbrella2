/** @file TypeDescr_ppdetail.hpp
 *
 *  Transitional compatibility header: legacy xo-indentlog ppdetail<>
 *  specializations for TypeDescrBase / TypeDescr.
 *
 *  TypeDescr.hpp itself is now ppsink-only; see TypeDescr_pp.hpp for the
 *  native Prettifier<>.  Code not yet migrated off legacy indentlog that
 *  pretty-prints a TypeDescr should include THIS header.
 *
 *  NB: xo-reflect no longer declares an xo-indentlog dependency, so a
 *  consumer that includes this header must declare one itself.  Nothing
 *  inside xo-reflect includes it, which is why the subsystem can be
 *  ppsink-only while still shipping this.  Retire it once the consuming
 *  subsystems migrate.
 *
 *  Renders from the public accessors rather than forwarding to
 *  TypeDescrBase::pretty(), which now takes a PpSink & and no longer fits the
 *  legacy two-pass ppindentinfo protocol.
 *
 *  DRIFT WARNING: the field list below duplicates the one in
 *  TypeDescrBase::pretty() (TypeDescr.cpp).  They render the same struct two
 *  ways; keep them in step, or the legacy and ppsink outputs diverge silently.
 **/

#pragma once

#include "TypeDescr.hpp"

namespace xo {
    namespace print {
        template <>
        struct ppdetail<xo::reflect::TypeDescrBase> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     const xo::reflect::TypeDescrBase & td) {
                /* refrtag captures BY REFERENCE, and three of these accessors
                 * return by value -- bind them to locals so the referents
                 * outlive the pretty_struct() call.
                 */
                const auto id = td.id();
                const bool complete = td.complete_flag();
                const auto metatype = td.metatype();

                return ppii.pps()->pretty_struct
                    (ppii,
                     "TypeDescr",
                     refrtag("id", id),
                     refrtag("canonical_name", td.canonical_name()),
                     refrtag("complete", complete),
                     refrtag("metatype", metatype));
            }
        };

        template <>
        struct ppdetail<xo::reflect::TypeDescr> {
            static bool print_pretty(const ppindentinfo & ppii,
                                     xo::reflect::TypeDescr td) {
                return (td
                        ? ppdetail<xo::reflect::TypeDescrBase>::print_pretty(ppii, *td)
                        : true);
            }
        };
    } /*namespace print*/
} /*namespace xo*/

/* end TypeDescr_ppdetail.hpp */
