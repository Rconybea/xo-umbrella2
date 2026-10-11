/** @file MapTdx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "xo/reflect/EstablishTypeDescr.hpp"
#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/TypeDescrExtra.hpp"
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <vector>

namespace xo {
    namespace reflect {
        /** @brief extra type-associated information for an associative
         *  container: std::map, std::unordered_map.
         *
         *  Its metatype is mt_map.  A consumer walks its entries --
         *  (key, value), each in place -- through visit_entries(), in KEY
         *  ORDER, so output is deterministic for an unordered map too.
         *
         *  Not by index: a node container has none, so child_tp(i) (the
         *  i'th value, in key order) costs a walk -- there for generic
         *  traversal, not for a consumer that visits every entry.
         *
         *  Keys and values are held inline: owning
         *  (TypeDescrExtra::child_edge_ownership()).  A pointer value
         *  (std::unique_ptr, rp) reaches its pointee by its own edge.
         *
         *  Reached through TypeDescrExtra::map_info()
         *  (TypeDescr::is_map()).  See .xo-backlog/xo-reflect/issues/05.
         **/
        class MapTdx : public TypeDescrExtra {
        public:
            /** called with each entry's key and value, in place **/
            using EntryFn = std::function<void (TaggedPtr key, TaggedPtr value)>;

        public:
            /** the key type **/
            virtual TypeDescr key_td() const = 0;
            /** the value (mapped) type **/
            virtual TypeDescr value_td() const = 0;
            /** call @p fn with each entry of the map at @p object, in key
             *  order: a std::map's own; an unordered map's sorted by
             *  std::less of its key type, when that has one, else in its
             *  iteration order
             **/
            virtual void visit_entries(void * object, EntryFn const & fn) const = 0;

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_map; }
            virtual uint32_t n_child_fixed() const override { return 0; /*unknown*/ }
            /** the i'th value, in key order: a walk (see class comment) **/
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            /** the value type, any i **/
            virtual TypeDescr fixed_child_td(uint32_t i) const override;
            /* (forbidden) */
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual const MapTdx * map_info() const override { return this; }
        }; /*MapTdx*/

        // ----- StdMapTdx -----

        /** a MapTdx for std::map<K, V, ..> and std::unordered_map<K, V, ..>.
         *
         *  require:
         *  - MapT::key_type, MapT::mapped_type, MapT::value_type
         *  - MapT.size(), iteration over std::pair<const K, V>
         *  An ordered map has MapT::key_compare: its order is key order.
         **/
        template <typename MapT>
        class StdMapTdx : public MapTdx {
        public:
            using target_t = MapT;
            using key_type = typename MapT::key_type;
            using mapped_type = typename MapT::mapped_type;
            using entry_type = typename MapT::value_type;

            /** true for std::map: iterates in key order already **/
            static constexpr bool c_ordered = requires { typename MapT::key_compare; };
            /** true iff keys compare with <, so an unordered map can be sorted.
             *  A nested requirement, not `{ a < b } -> concept`: clang 19's
             *  -ast-dump (xo-type-src-map) segfaults on a failed compound
             *  requirement
             **/
            static constexpr bool c_sortable
                = requires (key_type const & a, key_type const & b) {
                    requires std::convertible_to<decltype(a < b), bool>;
                };

        public:
            static std::unique_ptr<StdMapTdx> make() {
                return std::unique_ptr<StdMapTdx>(new StdMapTdx());
            } /*make*/

            virtual TypeDescr key_td() const override {
                return EstablishTypeDescr::establish<key_type>();
            }

            virtual TypeDescr value_td() const override {
                return EstablishTypeDescr::establish<mapped_type>();
            }

            virtual void visit_entries(void * object, EntryFn const & fn) const override {
                target_t * m = reinterpret_cast<target_t *>(object);

                if constexpr (c_ordered || !c_sortable) {
                    for (entry_type & e : *m)
                        visit_entry(e, fn);
                } else {
                    /* an unordered map: sort pointers to its entries */
                    std::vector<entry_type *> v;
                    v.reserve(m->size());

                    for (entry_type & e : *m)
                        v.push_back(&e);

                    std::sort(v.begin(), v.end(),
                              [](entry_type const * x, entry_type const * y) {
                                  return std::less<key_type>()(x->first, y->first);
                              });

                    for (entry_type * e : v)
                        visit_entry(*e, fn);
                }
            } /*visit_entries*/

            // ----- Inherited from TypeDescrExtra -----

            virtual uint32_t n_child(void * object) const override {
                return reinterpret_cast<target_t *>(object)->size();
            }

        private:
            static void visit_entry(entry_type & e, EntryFn const & fn) {
                /* const_cast: the key is const in the entry; TaggedPtr
                 * carries no const (see RawPointerTdx::child_tp).
                 * Reflection only reads through it
                 */
                fn(establish_most_derived_tp(const_cast<key_type *>(&e.first)),
                   establish_most_derived_tp(&e.second));
            }
        }; /*StdMapTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end MapTdx.hpp */
