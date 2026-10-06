/** @file EnumTdx.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace xo {
    namespace reflect {
        /** @brief extra type-associated information for a reflected enum:
         *  its enumerators, names and values.
         *
         *  An enum composes no other type, so its metatype is mt_atomic, like
         *  string, int, float and bool.  What it adds is the conversion
         *  between a value and an enumerator's name -- reached through
         *  TypeDescrExtra::enum_info() (TypeDescr::is_enum()).
         *
         *  Installed by EnumReflector (EnumReflector.hpp).  Type-erased: it
         *  reads and writes an enum object through two functions captured
         *  when it is made, which know the enum's underlying type.  Values
         *  travel as std::int64_t, so an enumerator of an unsigned 64-bit
         *  enum above INT64_MAX does not round-trip.
         *
         *  See .xo-backlog/xo-reflect/issues/06.
         **/
        class EnumTdx : public TypeDescrExtra {
        public:
            /** one enumerator: its value and its name **/
            struct Enumerator {
                std::int64_t value_;
                std::string name_;
            };

            /** an enum object's value, from its address **/
            using LoadFn = std::int64_t (*)(void const * object);
            /** set an enum object, at its address, to a value **/
            using StoreFn = void (*)(void * object, std::int64_t value);

        public:
            /** enumerators @p enum_v, in declaration order; objects read and
             *  written through @p load, @p store
             **/
            static std::unique_ptr<EnumTdx> make(std::vector<Enumerator> enum_v,
                                                 LoadFn load,
                                                 StoreFn store);

            /** number of reflected enumerators **/
            std::size_t n_enumerator() const { return enum_v_.size(); }
            /** the i'th enumerator's name, in declaration order **/
            std::string const & enumerator_name(std::size_t i) const { return enum_v_.at(i).name_; }
            /** the i'th enumerator's value **/
            std::int64_t enumerator_value(std::size_t i) const { return enum_v_.at(i).value_; }

            /** the value of the enum object at @p object, as an integer **/
            std::int64_t value_of(void const * object) const { return load_(object); }

            /** the name of the enumerator whose value is @p object's -- the
             *  first such, if enumerators share a value; nullptr if none has
             *  it (an unreflected value, or a combination of flags)
             **/
            std::string const * name_of(void const * object) const;

            /** set the enum object at @p object to the enumerator named
             *  @p name; false, leaving it unchanged, if there is none
             **/
            bool assign_from_name(std::string_view name, void * object) const;

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_atomic; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual const TypeDescrBase * fixed_child_td(uint32_t i) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual const EnumTdx * enum_info() const override { return this; }

        private:
            EnumTdx(std::vector<Enumerator> enum_v, LoadFn load, StoreFn store)
                : enum_v_{std::move(enum_v)}, load_{load}, store_{store} {}

        private:
            /** enumerators, in declaration order **/
            std::vector<Enumerator> enum_v_;
            /** reads an enum object **/
            LoadFn load_ = nullptr;
            /** writes an enum object **/
            StoreFn store_ = nullptr;
        }; /*EnumTdx*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end EnumTdx.hpp */
