/** @file PrintJsonAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "PrintJsonConfig.hpp"
#include "PrintJson.hpp"
#include "xo/printjson/PrintJsonSingleton.hpp"
#include <xo/reflect/cx/ReflectAppcx.hpp>

namespace xo {
    /** @brief Application-level state for the printjson subsystem
     **/
    class PrintJsonAppcx {
    public:
        using PrintJson = xo::json::PrintJson;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization, from @p cfg
         *
         *  Primary driver for printjson init
         **/
        PrintJsonAppcx(const PrintJsonConfig & cfg,
                       ReflectAppcx & reflect_appcx);

        /** Template for printjson init. Works with AppConfig, AppContext
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        PrintJsonAppcx(Deps & deps,
                       const PrintJsonConfig & cfg)
            : PrintJsonAppcx(cfg, deps.template cx<S_reflect_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const PrintJsonConfig & config() const { return config_; }
        PrintJson * print_json() const { return print_json_.get(); }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  (placeholder. enable when PrintJson refactored to use DArena)
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** xo-printjson/ configuration **/
        PrintJsonConfig config_;

        /** json printers, indexed by type **/
        rp<PrintJson> print_json_;
    };

    template <>
    class SubsystemContext<S_printjson_tag> {
    public:
        using Type = PrintJsonAppcx;
    };
} /*namespace xo*/

/* end PrintJsonAppcx.hpp */
