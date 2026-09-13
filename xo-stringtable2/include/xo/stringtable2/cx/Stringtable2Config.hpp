/** @file Stringtable2Config.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/stringtable2/init_stringtable2.hpp"
#include <xo/subsys/AppContext.hpp>

namespace xo {
    /** @brief configuration for subsystem xo-stringtable2/
     *
     *  Empty, and expected to stay that way: this subsystem's one sized
     *  object, @ref xo::scm::StringTable, is NOT owned here.  A StringTable is
     *  constructed with an explicit capacity by whoever needs one -- e.g.
     *  ParserStateMachine holds one by value -- so its capacity is that
     *  owner's business, not the application's.
     **/
    class Stringtable2Config {
    public:
        Stringtable2Config() = default;
    };

    /** xo-stringtable2 contributes a trivial configuration and a context **/
    template <>
    class SubsystemConfig<S_stringtable2_tag> {
    public:
        using Type = Stringtable2Config;
    };
} /*namespace xo*/

/* end Stringtable2Config.hpp */
