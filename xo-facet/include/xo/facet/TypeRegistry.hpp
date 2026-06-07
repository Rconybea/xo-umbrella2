/** @file TypeRegistry.hpp
 *
 *  @brief Runtime facet implementation lookup
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "typeseq.hpp"
#include <xo/arena/DArenaVector.hpp>
#include <xo/indentlog/scope.hpp>
#include <utility>

namespace xo {
    namespace facet {

        /** @class TypeRegistry
         *
         *  @brief Runtime registry for types.
         *
         *  Just assigns ids and remembers names.
         *  Not a full reflection implementation
         **/
        class TypeRegistry {
        public:
            using ReprType = xo::mm::DArenaVector<typerecd>;
            using ArenaConfig = xo::mm::ArenaConfig;
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
            using typeseq = xo::reflect::typeseq;

            /** singleton instance.
             *  @p hint_max_capacity is a lower bound for registry capacity.
             *  Only honored the first time instance is called.
             **/
            static TypeRegistry & instance(uint32_t hint_max_capacity = 1024) {
                static TypeRegistry s_instance(hint_max_capacity);
                return s_instance;
            }

            /** Type-safe registration
             *
             *  Registers the compile-time FacetImplementation<AFacet, DRepr>
             *  for runtime lookup.
             *
             *  @tparam AFacet  abstract facet type
             *  @tparam DRepr   data representation type
             **/
            template <typename T>
            static void register_type() {
                typerecd r = typerecd::recd<T>();

                instance()._register_type(r);
            }

            static std::string_view id2name(typeseq id) noexcept {
                return instance()._id2name(id);
            }

            /** Number of registered (facet, repr) pairs **/
            std::size_t size() const { return registry_.size(); }

            /** visit memory pools owned by facet registry **/
            void visit_pools(const MemorySizeVisitor & visitor) {
                registry_.visit_pools(visitor);
            }

            /** Check if type is registered **/
            bool contains(typeseq id) const
            {
                if ((0 <= id.seqno())
                    && (id.seqno() < static_cast<int32_t>(registry_.size())))
                {
                    return (registry_.at(id.seqno()).seqno() == id.seqno());
                }

                return false;
            }

            void dump(std::ostream * p_out) const {
                (*p_out) << std::endl;
                (*p_out) << "<TypeRegistry" << std::endl;
                for (const auto & item : registry_) {
                    (*p_out)
                        << "  [" << item.seqno() << "]"
                        << " -> " << item.name() << std::endl;
                }
                (*p_out) << ">" << std::endl;
            }

        private:
            /** Register a facet implementation (type-erased)
             *
             *  @param facet_id  typeseq for abstract facet (e.g., APrintable)
             *  @param repr_id   typeseq for data representation (e.g., DFloat)
             *  @param impl      pointer to stateless implementation instance
             **/
            void _register_type(const typerecd & recd)
            {
                if ((recd.seqno() >= 0)
                    && (static_cast<int32_t>(registry_.size()) <= recd.seqno()))
                {
                    registry_.resize(recd.seqno() + 1);
                }

                registry_.at(recd.seqno()) = recd;
            }

            /** Get typename from @p id.
             **/
            std::string_view _id2name(typeseq id) const
            {
                if ((0 <= id.seqno())
                    && (static_cast<std::size_t>(id.seqno()) < registry_.size()))
                {
                    return registry_.at(id.seqno()).name();
                }

                return typerecd::sentinel().name();
            }

        private:
            TypeRegistry(uint32_t hint_max_capacity)
                : registry_(ReprType::map(ArenaConfig()
                                          .with_name("types")
                                          .with_size(hint_max_capacity
                                                     * sizeof(typerecd))))
                {}

            /** runtime lookup table (AFacet,DRepr) -> impl **/
            ReprType registry_;
        };

    } /*namespace facet*/
} /*namespace xo*/

/* end TypeRegistry.hpp */
