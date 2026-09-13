/** @file ReflectAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/reflect/cx/ReflectConfig.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include <xo/indentlog2/cx/Indentlog2Appcx.hpp>
#include <xo/arena/MemorySizeInfo.hpp>

namespace xo {
    /** @brief Application-level state for the reflect subsystem
     **/
    class ReflectAppcx {
    public:
        using TypeDescrTable = xo::reflect::TypeDescrTable;
        using MemorySizeVisitor = xo::mm::MemorySizeVisitor;

    public:
        /** non-template initialization from @p cfg
         *
         *  Primary driver for reflect init
         **/
        ReflectAppcx(const ReflectConfig & cfg,
                     const Indentlog2Appcx & indentlog2_cx);

        /** Template for reflect init. Works with AppConfig, AppContext
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        ReflectAppcx(Deps & deps,
                     const ReflectConfig & cfg) : ReflectAppcx(cfg, deps.template cx<S_indentlog2_tag>()) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const ReflectConfig & config() const { return config_; }
        TypeDescrTable * type_table() const { return type_table_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *
         *  Placeholder: reports nothing, because reflect/ has no pools to
         *  report yet.  Becomes meaningful when TypeDescrTable is represented
         *  with a DArena.  Present now so the shape matches every other
         *  Appcx -- a caller walking the stack need not special-case this one.
         **/
        void visit_pools(const MemorySizeVisitor &) const {}

    private:
        /** ensure low-level subsystem initialization **/
        InitEvidence init_evidence_;

        /** reflect/ configuration **/
        ReflectConfig config_;

        /** Table of reflected types.
         *
         *  Same as TypeDescrTable::instance();
         *  forced by Reflect::reflect<T>() interface
         **/
        TypeDescrTable * type_table_ = nullptr;

        /** xo-indentlog2/ context **/
        const Indentlog2Appcx & indentlog2_appcx_;
    };

    template <>
    class SubsystemContext<S_reflect_tag> {
    public:
        using Type = ReflectAppcx;
    };
} /*namespace xo*/

/* end ReflectAppcx.hpp */
