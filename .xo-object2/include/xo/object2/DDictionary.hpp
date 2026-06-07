/** @file DDictionary.hpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#pragma once

#include "DArray.hpp"
#include "DString.hpp"
#include <xo/alloc2/GCObject.hpp>
#include <xo/alloc2/GCObjectVisitor.hpp>
#include <xo/alloc2/Allocator.hpp>
#include <xo/facet/obj.hpp>
#include <xo/indentlog/print/ppindentinfo.hpp>
#include <concepts>
#include <cstdint>

namespace xo {
    namespace scm {
        /** @class DStruct
         *  @brief Polymorphic in-memory key-value store with gc hooks
         *
         *  Small dictionary implementation for Schematika.
         *  O(n) lookup, at least for now.  Not typed.  Keys are strings,
         *  so functionally equivalent to python dictionaries.
         **/
        class DDictionary {
        public:
            /** @defgroup ddictionary-types type traits **/
            ///@{

            /** type for array size **/
            using size_type = std::uint32_t;
            /** xo allocator facet **/
            using AAllocator = xo::mm::AAllocator;
            /** gc-centric object visitor **/
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            /** gc-aware object facet **/
            using AGCObject = xo::mm::AGCObject;
            /** color for gco visitor **/
            using VisitReason = xo::mm::VisitReason;
            /** pretty-printer state for APrintable **/
            using ppindentinfo = xo::print::ppindentinfo;
            /** canonical type representing a key-value pair **/
            using pair_type = std::pair<const DString *, obj<AGCObject>>;

            /** shim to represent result of expression like @c dict[key]
             **/
            template <typename DictPtr>
            class LValue {
            public:
                LValue(DictPtr d, const DString * key) : dict_{d}, key_{key} {}

                bool is_sentinel() const noexcept { return dict_ == nullptr; }
                bool is_valid() const noexcept { return dict_ != nullptr; }

                obj<AGCObject> & operator=(obj<AGCObject> & rvalue) {
                    return dict_->upsert(key_, rvalue);
                }

                operator std::optional<obj<AGCObject>>() const noexcept {
                    return dict_->lookup(key_);
                }

            private:
                /** sentinel LValue represented by null ptr here **/
                DictPtr dict_ = nullptr;
                /** sentinel LValue has null ptr here **/
                const DString * key_ = nullptr;
            };

            ///@}
            /** @defgroup ddictionary-ctors constructors **/
            ///@{

            /** default ctor. zero capacity sentinel **/
            DDictionary(DArray * keys, DArray * values);

            /** create empty array with space for @p cap elements
             *  using memory from allocator @p mm.
             *  Nullptr if space exhausted
             **/
            static DDictionary * empty(obj<AAllocator> mm,
                                       size_type cap);

            /** create copy of @p src using memory from @p mm
             *  with capacity for @p new_cap elements
             **/
            static DDictionary * copy(obj<AAllocator> mm,
                                      DDictionary * src,
                                      size_type new_cap);

            /** create dictionary containing elements @p kv_pairs, using memory from @p mm.
             *  Nullptr if space exhausted.
             *
             *  Use:
             *    auto kv1 = std::make_pair<const DString *, obj<AGCObject>>(...);
             *    auto kv2 = std::make_pair<const DString *, obj<AGCObject>>(...);
             *    Ddictionary * v = DDictionary::make(mm, kv1, kv2, ..);
             **/
            template <typename... Args>
            requires (std::same_as<Args, std::pair<const DString*, obj<AGCObject>>> && ...)
            static DDictionary * make(obj<AAllocator> mm, Args... args);

            ///@}
            /** @defgroup ddictionary-access acecss methods **/
            ///@{
            /** true iff array is empty **/
            bool is_empty() const noexcept { return keys_->size() == 0; }
            /** array capacity **/
            size_type capacity() const noexcept { return keys_->capacity(); }
            /** current dictionary size (number of key-value pairs) **/
            size_type size() const noexcept { return keys_->size(); }

            /** return value associated with @p key, if key is present **/
            std::optional<obj<AGCObject>> lookup(const DString * key) const noexcept;
            /** return value associated with @p key, if key is present **/
            std::optional<obj<AGCObject>> lookup_cstr(const char * key) const noexcept;

            /** return element @p key-value pair at position @p index (0-based) **/
            std::pair<const DString *, obj<AGCObject>> at_index(size_type index) const;
            /** return @p i'th key. O(1) **/
            const DString * key_at_index(size_type i) const;
            /** return @p i'th value. O(1)  **/
            obj<AGCObject> value_at_index(size_type i) const;


            auto operator[](const DString * key) const noexcept { return LValue<decltype(this)>(this, key); }
            auto operator[](const DString * key) noexcept { return LValue<decltype(this)>(this, key); }

            ///@}
            /** @defgroup ddictionary-iterators iterators **/
            ///@{

            ///@}
            /** @defgroup ddictionary-assign assignment **/
            ///@{

            /** update key-value pair @p kvpair in-place,
             *  provide key is already present.
             *
             *  @return true if key-value pair updated; false if key not found
             **/
            bool try_update(obj<AAllocator> mm, const pair_type & kvpair);

            /** update key-value pair for existing @p key to map to @p value.
             *  false if @p key not already present.
             **/
            bool try_update_cstr(obj<AAllocator> gc, const char * key, obj<AGCObject> value);

            /** convenience method:
             *  try_upsert pair (k, @p value), after boxing c-style string @p key with @p mm to get k
             **/
            bool try_upsert_cstr(obj<AAllocator> mm, const char * key, obj<AGCObject> value);

            /** convenience method:
             *  upsert pair (k, @p value), after boxing c-style string @p key with @p mm to get k
             **/
            bool upsert_cstr(obj<AAllocator> mm, const char * key, obj<AGCObject> value);

            /** upsert key-value pair @p kvpair into dictionary.
             *  If key kvpair.first not already present, add it.
             *  In either case replace/establish associated value with kvpair.second.
             *
             *  False if dictionary already at capacity
             **/
            bool try_upsert(obj<AAllocator> gc, const pair_type & kvpair);

            /** upsert key-value pair @p kvpair into dictionary.
             *  If at capacity, expand capacity, getting new memory from @p mm.
             *
             *  False iff upsert failed because allocator memory exhausted
             **/
            bool upsert(obj<AAllocator> mm, const pair_type & kvpair);

            ///@}
            /** @defgroup ddictionary-general general methods **/
            ///@{

            /** reduce array capacity to current array size
             *
             *  note: with X1Collector, capacity is reduced but memory not recycled
             *        until next collection
             **/
            void shrink_to_fit() noexcept;

            ///@}
            /** @defgroup ddictionary-conversion-operators conversion operators **/
            ///@{

            ///@}
            /** @defgroup ddictionary-sequence-methods **/
            ///@{

            ///@}
            /** @defgroup ddictionary-printable-methods **/
            ///@{

            /** pretty-printing support **/
            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup ddictionary-gcobject-methods **/
            ///@{
            /** return shallow copy of this array, using memory from @p mm **/
            DDictionary * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            /** forward elements to @p gc to-space; replace originals with forwarding pointers **/
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;
            ///@}

        private:
            /** @defgroup ddictionary-impl-methods implementation methods **/
            ///@{
            /** append {key, value} pair @p kv_pair to this dictionary
             *  Require: @p kv_pair.first not already present in @ref keys_
             **/
            bool _append_kv_aux(obj<AAllocator> mm, const pair_type & kv_pair);
            ///@}

        private:
            /** @defgroup ddictionary-instance-variables instance variables **/
            ///@{

            /** dictionary keys.  These will be strings **/
            DArray * keys_;
            /** dictionary values.  values_[i] associates with keys_[i] **/
            DArray * values_;

            ///@}
        };

        template <typename... Args>
        requires (std::same_as<Args, std::pair<const DString *, obj<DDictionary::AGCObject>>> && ...)
        DDictionary *
        DDictionary::make(obj<AAllocator> mm, Args... args)
        {
            DDictionary * result = empty(mm, sizeof...(args));
            if (result) {
                (result->upsert(mm, args), ...);
            }
            return result;
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DDictionary.hpp */
