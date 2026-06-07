/** @file PrimitiveRegistry.hpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#pragma once

#include "Procedure.hpp"
#include <xo/alloc2/Allocator.hpp>
#include <xo/reflect/TypeDescr.hpp>
#include <functional>

namespace xo {
    namespace scm {
        /** partition primitives based on scope.
         *  Each primitive associates with exactly one flag value.
         *  Flags also operate as bitmasks when calling
         *  @ref PrimitiveRegistry::install_primitives()
         **/
        enum class InstallFlags {
            f_none = 0x0,

            /** mandatory primitives, necessary for operator support,
             *  e.g. _add needed to implement infix +
             **/
            f_essential = 0x1,
            f_generalpurpose = 0x2,

            /** all primitives **/
            f_all = f_essential | f_generalpurpose,
        };

        inline InstallFlags operator&(InstallFlags x, InstallFlags y) {
            return InstallFlags(static_cast<uint64_t>(x) & static_cast<uint64_t>(y));
        }

        /** provided by VSM to receive created primitives.
         *  InstallSink(pm) adopts a primitive
         **/
        using InstallSink = std::function<bool (std::string_view name,
                                                xo::reflect::TypeDescr fn_td,
                                                obj<AProcedure> pm,
                                                InstallFlags flags)>;

        /** @class PrimitiveRegistry
         *
         *  @brief Runtime registry for primitives
         *
         *  Singleton to remember setup code for known primitives.
         *  Use to gather primitives for installation in global
         *  environment for a virtual schematika machine (VSM)
         **/
        class PrimitiveRegistry {
        public:
            using AAllocator = xo::mm::AAllocator;

            /** provided by a subsystem that provides primitives.
             *  Allocates primitives using memory from mm, delivering them
             *  to InstallSink sink.
             **/
            using InstallSource = std::function<bool (obj<ARuntimeContext> rcx,
                                                      //obj<AAllocator> mm,
                                                      //StringTable * stbl,
                                                      InstallSink sink,
                                                      InstallFlags flags)>;

        public:
            /** singleton instance **/
            static PrimitiveRegistry & instance();

            template <typename PrimitiveRepr>
            static bool install_aux(InstallSink sink,
                                    PrimitiveRepr * pm,
                                    InstallFlags flags) {
                scope log(XO_DEBUG(false));

                if (flags != InstallFlags::f_none) {
                    log && log("create primitive", xtag("name", pm->name()));

                    return sink(pm->name(),
                                pm->fn_td(),
                                obj<AProcedure,PrimitiveRepr>(pm),
                                flags);
                } else {
                    log && log("skip primitive", xtag("name", pm->name()));

                    return true;
                }
            }

            /** remember primitive-factory @p source_fn **/
            void register_primitives(InstallSource source_fn);

            /** create primitives using memory from @p mm,
             *  with global strings in @p stbl.
             *  delivering each primitive to @p sink.
             **/
            bool install_primitives(obj<ARuntimeContext> rcx,
                                    //obj<AAllocator> mm,
                                    //StringTable * stbl,
                                    InstallSink sink,
                                    InstallFlags flags);


        private:
            /** a set of factories that create primitives **/
            std::vector<InstallSource> init_seq_v_;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end PrimitiveRegistry.hpp */
