/** @file Stringtable2Appcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/Stringtable2Appcx.hpp"

namespace xo {
    Stringtable2Appcx::Stringtable2Appcx(const Stringtable2Config & cfg,
                                         const FacetAppcx & /*facet_appcx*/)
        : init_evidence_{InitSubsys<S_stringtable2_tag>::require()},
          config_{cfg}
    {}

} /*namespace xo*/

/* end Stringtable2Appcx.cpp */
