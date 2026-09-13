/** @file Object2Appcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "Object2Config.hpp"
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
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
         *  Primary driver for printjson init
         **/
        Object2Appcx(const Object2Config & cfg,
                     PrintJsonAppcx & pjson_appcx);

        /** Template for object2 init. Works with AppConfig, AppContext
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        Object2Appcx(Deps & deps,
                     const Object2Config & cfg)
            : Object2Appcx(cfg, deps.template cx<S_printjson_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const Object2Config & config() const { return config_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  (not expecting to use this)
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** xo-printjson/ configuration **/
        Object2Config config_;
    };
} /*namespace xo*/

/* end Object2Appcx.hpp */
