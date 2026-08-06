/** @file quantity_iostream.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include "natural_unit_iostream.hpp"
#include "quantity.hpp"

namespace xo {
    namespace qty {
        template < auto NaturalUnit, typename Repr >
        inline std::ostream &
        operator<< (std::ostream & os,
                    const quantity<NaturalUnit, Repr> & x)
        {
            os << x.scale() << x.abbrev();
            return os;
        }

    } /*namespace qty*/

    /* NB there was a legacy xo-indentlog ppdetail<quantity<..>> here, declaring
     * quantity a print-atom by delegating to ppdetail_atomic<>.  Deleted, not
     * ported, for two reasons:
     *
     * 1. It was dead code.  It sat under #ifndef ppdetail_atomic, and
     *    xo-indentlog's ppdetail_atomic.hpp defines that macro
     *    unconditionally -- so the block never compiled in any build that saw
     *    indentlog.  Dropping the indentlog include is what "activated" it.
     * 2. ppsink needs no equivalent.  Its primary Prettifier<T> template is
     *    empty, so a type with no specialization already falls through to the
     *    string-like leaf and then to operator<< -- which is exactly what
     *    declaring it an atom was asking for.  operator<< is right above.
     */
} /*namespace xo*/

/** end quantity_iostream.hpp **/
