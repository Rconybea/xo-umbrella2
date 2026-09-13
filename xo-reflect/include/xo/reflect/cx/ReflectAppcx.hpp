/** @file ReflectAppcx.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "xo/reflect/cx/ReflectConfig.hpp"
#include "xo/reflect/TypeDescr.hpp"
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
        ReflectAppcx(const ReflectConfig & cfg);

        /** Template for reflect init. Works with AppConfig, AppContext
         *
         *  @p deps  contexts of the subsystems below this one.
         *  @p cfg   configuration for this subsystem
         **/
        template <typename Deps>
        ReflectAppcx(Deps & /*deps*/,
                     const ReflectConfig & cfg) : ReflectAppcx(cfg) {}

        InitEvidence init_evidence() const { return init_evidence_; }
        const ReflectConfig & config() const { return config_; }
        TypeDescrTable * type_table() const { return type_table_; }
        /** report memory consumption, one @ref MemorySizeInfo per pool.
         *  (placeholder, enable when reflect/ refactored to use DArena)
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
    };

    template <>
    class SubsystemContext<S_reflect_tag> {
    public:
        using Type = ReflectAppcx;
    };
} /*namespace xo*/

/* end ReflectAppcx.hpp */
