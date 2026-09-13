/** @file ReflectAppcx.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "cx/ReflectAppcx.hpp"

namespace xo {
    using xo::reflect::TypeDescrTable;

    ReflectAppcx::ReflectAppcx(const ReflectConfig & cfg)
        : init_evidence_{InitSubsys<S_reflect_tag>::require()},
          config_{cfg}
    {
        this->type_table_ = TypeDescrTable::instance();
    }


} /*namespace xo*/

/* end ReflectAppcx.cpp */
