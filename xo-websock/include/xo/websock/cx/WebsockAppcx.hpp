/** @file WebsockAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "WebsockConfig.hpp"
#include <xo/printjson/cx/PrintJsonAppcx.hpp>
#include <xo/subsys/AppContext.hpp>
#include <xo/subsys/Evidence.hpp>

namespace xo {
    /** @brief application-level state for the websock subsystem.
     *
     *  Establishing one registers xo-websock's json printers with the
     *  printjson context's PrintJson (xo/websock/websock_json.hpp).  A
     *  Webserver is made FROM a WebsockAppcx (Webserver::make), so a server
     *  cannot exist without its printers.  See
     *  .xo-backlog/xo-websock/issues/11.
     **/
    class WebsockAppcx {
    public:
        using CreationEvidence = Evidence<class WebsockAppcxCreated_tag>;
        using CreationEvp = EvidenceProvider<CreationEvidence>;
        using PrintJson = xo::json::PrintJson;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization, from @p cfg.
         *
         *  Primary driver for websock init.
         **/
        WebsockAppcx(const WebsockConfig & cfg,
                     const PrintJsonAppcx & printjson_appcx);

        /** Template for websock init.  Works with AppConfig, AppContext.
         *
         *  @p deps contexts of the subsystems below this one.
         *  @p cfg  configuration for this subsystem
         **/
        template <typename Deps>
        WebsockAppcx(Deps & deps,
                     const WebsockConfig & cfg)
            : WebsockAppcx(cfg, deps.template cx<S_printjson_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        CreationEvidence creation_evidence() const { return websock_evp_; }
        const WebsockConfig & config() const { return config_; }
        /** json printers, with xo-websock's installed; what servers made
         *  from this context print with
         **/
        rp<PrintJson> print_json() const { return print_json_; }

        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *
         *  Placeholder: xo-websock owns no pools.  Present so the shape
         *  matches every other Appcx.
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure low-level subsystem initialization **/
        InitEvidence init_evidence_;
        /** provides evidence that a WebsockAppcx has been created **/
        CreationEvp websock_evp_;
        /** xo-websock/ configuration **/
        WebsockConfig config_;
        /** from the printjson context; xo-websock's printers installed **/
        rp<PrintJson> print_json_;
    };

    template <>
    class SubsystemContext<S_websock_tag> {
    public:
        using Type = WebsockAppcx;
    };
} /*namespace xo*/

/* end WebsockAppcx.hpp */
