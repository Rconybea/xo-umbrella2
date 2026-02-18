/** @file FacetRegistry.hpp
 *
 *  @brief Runtime facet implementation lookup
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "TypeRegistry.hpp"
#include "facet_implementation.hpp"
//#include "typeseq.hpp"
#include "obj.hpp"
#include <xo/arena/DArenaHashMap.hpp>
#include <xo/indentlog/scope.hpp>
#include <xo/indentlog/print/tostr.hpp>
//#include <unordered_map>
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
            using MemorySizeVisitor = xo::mm::MemorySizeVisitor;
            using typeseq = xo::reflect::typeseq;
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

            /** singleton instance.
             *  @p hint_max_capacity is a lower bound for swiss hash map implementation.
             *  Only honored the first time instance is called.
             **/
            static FacetRegistry & instance(uint32_t hint_max_capacity = 1024) {
                static FacetRegistry s_instance(hint_max_capacity);
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
            template <typename AFacet, typename DRepr>
            static void register_impl() {
                static FacetImplType<AFacet, DRepr> impl;

                TypeRegistry::register_type<AFacet>();
                TypeRegistry::register_type<DRepr>();

                instance()._register_impl(typeseq::id<AFacet>(),
                                          typeseq::id<DRepr>(),
                                          &impl);
            }

            /** Convenience function for runtime lookup
             *
             *  @tparam AFacet  abstract facet type
             *  @param repr_id  typeseq for data representation
             *  @return pointer to AFacet implementation, or nullptr
             **/
            template <typename AFacet>
            static inline const AFacet * impl_for(typeseq repr_id) {
                return FacetRegistry::instance().lookup<AFacet>(repr_id);
            }

            /** Number of registered (facet, repr) pairs **/
            std::size_t size() const { return registry_.size(); }

            /** visit memory pools owned by facet registry **/
            void visit_pools(const MemorySizeVisitor & visitor) {
                registry_.visit_pools(visitor);
            }

            /** Check if implementation is registered **/
            bool contains(typeseq facet_id,
                          typeseq repr_id) const
            {
                return (registry_.find(key_type(facet_id, repr_id))
                        != registry_.end());
            }

            /** Type-safe lookup
             *
             *  @tparam AFacet  abstract facet type
             *  @param repr_id  typeseq for data representation
             *  @return pointer to AFacet implementation, or nullptr
             **/
            template <typename AFacet>
            const AFacet * lookup(typeseq repr_id) const {
                return static_cast<const AFacet *>
                                      (this->_lookup(typeseq::id<AFacet>(), repr_id));
            }

            /** Runtime polymorphism:
             *  Switch @param from from interface @tp AFrom to interface @tp ATo.
             *
             *  Use:
             *    obj<AFoo> foo
             *      = ...;  // Foo instance with variant impl
             *    obj<ABar> bar
             *      = FacetRegistry::instance().variant<ABar,AFoo>(foo);
             *
             *    // exception thrown if bar has null data
             *
             *    assert(bar);
             **/
            template <typename ATo, typename AFrom>
            obj<ATo> variant(obj<AFrom> from) {
                auto retval = try_variant<ATo>(from);

                if (!retval)
                    throw std::runtime_error(tostr("FacetRegistry::try_variant failed",
                                                   xtag("AFrom.tseq", typeseq::id<AFrom>()),
                                                   xtag("ATo.tseq", typeseq::id<ATo>()),
                                                   xtag("DRepr", from._typeseq())));

                return retval;
            }

            /** Runtime polymorphism:
             *  Switch @param from from interface @tp AFrom to interface @tp ATo.
             *
             *  Use:
             *    obj<AFoo> foo
             *      = ...;  // Foo instance with variant impl
             *    obj<ABar> bar
             *      = FacetRegistry::instance().try_variant<ABar,AFoo>(foo);
             *    if (bar) {
             *       // success
             *    } else {
             *       // foo::DataType doesn't appear to support ABar
             *    }
             **/
            template <typename ATo, typename AFrom>
            obj<ATo> try_variant(obj<AFrom> from) noexcept {
                return try_variant<ATo>(from._typeseq(), from.data());
            }

            /** Runtime polymorphism:
             *  Create variant from representation @p data
             *  with actual type @p repr_id.
             *
             *  Use:
             *    obj<AFoo> foo = ...;  // Foo instance with variant impl
             *    obj<ABar> bar
             *      = FacetRegistry::variant<ABar>(foo._typeseq(), foo.opaque_data());
             **/
            template <typename AFacet>
            obj<AFacet> try_variant(typeseq repr_id, void * data) noexcept {
                const AFacet * iface = this->lookup<AFacet>(repr_id);

                if (iface)
                    return obj<AFacet>(iface, data);
                else
                    return obj<AFacet>();
            }

            void dump(std::ostream * p_out) {
                (*p_out) << std::endl;
                (*p_out) << "<FacetRegistry" << std::endl;
                for (auto & kv : registry_) {
                    (*p_out)
                    << "  [" << kv.first.first << "," << kv.first.second << "]"
                    << " -> " << kv.second << std::endl;
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
                scope log(XO_DEBUG(false));
                log && log(xtag("facet_id", facet_id),
                           xtag("repr_id", repr_id));

                auto ix = registry_.find(key_type(facet_id, repr_id));

                const void *retval = nullptr;

                if (ix != registry_.end())
                    retval = ix->second;

                log && log(xtag("retval", retval));

                return retval;
            }

        private:
            FacetRegistry(uint32_t hint_max_capacity)
              : registry_("facets", hint_max_capacity, false /*!debug_flag*/) {}

            /** runtime lookup table (AFacet,DRepr) -> impl **/
            xo::map::DArenaHashMap<key_type, const void *, KeyHash> registry_;
        };

        // Deferred definitioon of obj<AFacet,DRepr>::to_facet(),
        // since implementation requires FacetRegistry
        //
        template <typename AFacet, typename DRepr>
        template <typename AOther>
        obj<AOther,DRepr>
        obj<AFacet,DRepr>::to_facet()
        {
            if (this->data()) {
                if constexpr (std::is_same_v<DRepr, DVariantPlaceholder>) {
                    // return type has type-erased data
                    return FacetRegistry::instance().variant<AOther,AFacet>(*this);
                } else {
                    // return type has known data
                    return obj<AOther,DRepr>(this->data());
                }
            } else {
                return obj<AOther,DRepr>();
            }
        }

        // Deferred definitioon of obj<AFacet,DRepr>::to_facet(),
        // since implementation requires FacetRegistry
        //
        template <typename AFacet, typename DRepr>
        template <typename AOther>
        obj<AOther,DRepr>
        obj<AFacet,DRepr>::try_to_facet() noexcept
        {
            if (this->data()) {
                if constexpr (std::is_same_v<DRepr, DVariantPlaceholder>) {
                    // return type has type-erased data
                    return FacetRegistry::instance().try_variant<AOther,AFacet>(*this);
                } else {
                    // return type has known data
                    return obj<AOther,DRepr>(this->data());
                }
            } else {
                return obj<AOther,DRepr>();
            }
        }

    } /*namespace facet*/
} /*namespace xo*/

/* end FacetRegistry.hpp */
