/** @file Object2Appcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "Object2Config.hpp"
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/stringtable2/cx/Stringtable2Appcx.hpp>
//#include <xo/reflect/cx/ReflectAppcx.hpp>

namespace xo {
    /** @brief Application-level state for the printjson subsystem
     **/
    class Object2Appcx {
    public:
        using PrintJson = xo::json::PrintJson;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization, from @p cfg
         *
         *  Primary driver for object2 init.
         *
         *  @p pjson_appcx.  Used: SetupObject2 registers object2's json
         *  printers into this context's PrintJson.
         *
         *  @p stringtable2_appcx.  Not used directly; proof of work.  object2
         *  names DString/DUniqueString, whose facet implementations are
         *  registered by SetupStringtable2 -- so object2's own facet
         *  registration is only complete if that subsystem's ran first.
         **/
        Object2Appcx(const Object2Config & cfg,
                     const PrintJsonAppcx & pjson_appcx,
                     const Stringtable2Appcx & stringtable2_appcx);

        /** Template for object2 init. Works with AppConfig, AppContext
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        Object2Appcx(Deps & deps,
                     const Object2Config & cfg)
            : Object2Appcx(cfg,
                           deps.template cx<S_printjson_tag>(),
                           deps.template cx<S_stringtable2_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const Object2Config & config() const { return config_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  (not expecting to use this)
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** xo-object2/ configuration **/
        Object2Config config_;
    };

    template <>
    class SubsystemContext<S_object2_tag> {
    public:
        using Type = Object2Appcx;
    };
} /*namespace xo*/

/* end Object2Appcx.hpp */
