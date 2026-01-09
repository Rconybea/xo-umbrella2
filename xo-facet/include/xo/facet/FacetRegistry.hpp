/** @file FacetRegistry.hpp
 *
 *  @brief Runtime facet implementation lookup
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "facet_implementation.hpp"
#include "typeseq.hpp"
#include <unordered_map>
#include <utility>

namespace xo {
    namespace facet {

        /** @class FacetRegistry
         *
         *  @brief Runtime registry for facet implementations
         *
         *  Enables late-binding between facets. For example, given an
         *  obj<AGCObject> with unknown concrete type at compile time,
         * look up the APrintable implementation at runtime.
         *
         *  Example:
         *    // Registration (at initialization time)
         *    FacetRegistry::instance().register_impl<APrintable, DFloat>();
         *    FacetRegistry::instance().register_impl<APrintable, DList>();
         *
         *    // Lookup (at runtime)
         *    typeseq repr_id = obj->_typeseq();
         *    const APrintable* printable = runtime_impl_for<APrintable>(repr_id);
         *    printable->print(obj.data(), out);
         **/
        class FacetRegistry {
        public:
            using key_type = std::pair<typeseq, typeseq>;

            /** hash function for key_type **/
            struct KeyHash {
                std::size_t operator()(const key_type & k) const noexcept {
                    // combine the two seqno values
                    std::size_t h1 = std::hash<int32_t>{}(k.first.seqno());
                    std::size_t h2 = std::hash<int32_t>{}(k.second.seqno());
                    return h1 ^ (h2 << 1);
                }
            };

            /** singleton instance **/
            static FacetRegistry & instance() {
                static FacetRegistry s_instance;
                return s_instance;
            }

            /** Number of registered (facet, repr) pairs **/
            std::size_t size() const { return registry_.size(); }

            /** Check if implementation is registered **/
            bool contains(typeseq facet_id,
                          typeseq repr_id) const
            {
                return registry_.find(key_type(facet_id, repr_id)) != registry_.end();
            }

            /** Type-safe registration
             *
             *  Registers the compile-time FacetImplementation<AFacet, DRepr>
             *  for runtime lookup.
             *
             *  @tparam AFacet  abstract facet type
             *  @tparam DRepr   data representation type
             **/
            template <typename AFacet, typename DRepr>
            void register_impl() {
                static FacetImplType<AFacet, DRepr> impl;

                this->_register_impl(typeseq::id<AFacet>(),
                                     typeseq::id<DRepr>(),
                                     &impl);
            }

            /** Type-safe lookup
             *
             *  @tparam AFacet  abstract facet type
             *  @param repr_id  typeseq for data representation
             *  @return pointer to AFacet implementation, or nullptr
             **/
            template <typename AFacet>
            const AFacet * lookup(typeseq repr_id) const {
                return static_cast<const AFacet *>(this->_lookup(typeseq::id<AFacet>(), repr_id));
            }

        private:
            /** Register a facet implementation (type-erased)
             *
             *  @param facet_id  typeseq for abstract facet (e.g., APrintable)
             *  @param repr_id   typeseq for data representation (e.g., DFloat)
             *  @param impl      pointer to stateless implementation instance
             **/
            void _register_impl(typeseq facet_id,
                               typeseq repr_id,
                               const void * impl)
            {
                registry_[key_type(facet_id, repr_id)] = impl;
            }

            /** Lookup facet implementation (type-erased)
             *
             *  @return pointer to implementation, or nullptr if not registered
             **/
            const void * _lookup(typeseq facet_id,
                                 typeseq repr_id) const
            {
                auto ix = registry_.find(key_type(facet_id, repr_id));

                if (ix == registry_.end())
                    return nullptr;
                else
                    return ix->second;
            }

        private:
            FacetRegistry() = default;

            std::unordered_map<key_type, const void *, KeyHash> registry_;
        };

        /** Convenience function for runtime lookup
         *
         *  @tparam AFacet  abstract facet type
         *  @param repr_id  typeseq for data representation
         *  @return pointer to AFacet implementation, or nullptr
         **/
        template <typename AFacet>
        inline const AFacet * runtime_impl_for(typeseq repr_id) {
            return FacetRegistry::instance().lookup<AFacet>(repr_id);
        }

        /** Convenience function for registration
         *
         *  @tparam AFacet  abstract facet type
         *  @tparam DRepr   data representation type
         **/
        template <typename AFacet, typename DRepr>
        inline void register_facet_impl() {
            FacetRegistry::instance().register_impl<AFacet, DRepr>();
        }

    } /*namespace facet*/
} /*namespace xo*/

/* end FacetRegistry.hpp */
