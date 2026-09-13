/** @file ReflectAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/ReflectAppcx.hpp"

namespace xo {
    using xo::reflect::TypeDescrTable;

    ReflectAppcx::ReflectAppcx(const ReflectConfig & cfg,
                               const Indentlog2Appcx & indentlog2_cx)
        : init_evidence_{InitSubsys<S_reflect_tag>::require()},
          config_{cfg},
          indentlog2_appcx_{indentlog2_cx}
    {
        this->type_table_ = TypeDescrTable::instance();
    }


} /*namespace xo*/

/* end ReflectAppcx.cpp */
