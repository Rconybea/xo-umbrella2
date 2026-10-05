/** @file JsonMembers.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "PrintJson.hpp"
#include "type_keys.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <xo/refcnt/Refcounted.hpp>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace xo {
    namespace json {
        namespace detail {
            /** the type whose printability decides whether a member of type T
             *  can be printed: T itself, or what an rp<>, a raw pointer or a
             *  std::vector holds -- those are reflected generically, so
             *  checking them would check nothing
             **/
            template <typename T>
            struct member_target { using type = std::remove_cv_t<T>; };

            template <typename T>
            struct member_target<ref::intrusive_ptr<T>> : member_target<T> {};

            template <typename T>
            struct member_target<T *> : member_target<T> {};

            template <typename T>
            struct member_target<std::vector<T>> : member_target<T> {};
        }

        /** @brief writes an object's "_members_" array, for a JsonPrinter that
         *  opts in to showing chosen C++ members:
         *
         *    "_members_": [{"_name_": .., "_canonical_type_": .., "_short_type_": ..,
         *                   "_metatype_": .., "_value_": ..}, ..]
         *
         *  _canonical_type_, _short_type_ name the member's DECLARED type
         *  (see type_keys);
         *  _metatype_ that type's xo-reflect metatype (atomic, pointer,
         *  vector, struct, function -- metatype2str); _value_
         *  is printed by PrintJson as any value would be -- so an rp<T> or T*
         *  reflects as its pointee's actual type, and a nested object carries
         *  its own _members_ if its printer writes them.
         *
         *  A member whose type cannot be printed -- neither a json printer
         *  nor a complete reflected struct -- is written without a value:
         *    {"_name_": .., "_canonical_type_": .., "_short_type_": .., "_metatype_": ..,
         *     "_error_": "type not reflected: X"}
         *  so one omission does not spoil the rest of the output.
         *
         *  Use, inside a JsonPrinter's print_json, after the printer's own
         *  keys:
         *
         *    JsonMembers mem(state);                 // writes , "_members_": [
         *    mem.member("url_router_", x->url_router_);
         *    mem.member_as<std::atomic<int>>("port_", x->port_.load());
         *    mem.member_ref<rp<Sender>>("sender_", x->sender_.get());  // printed elsewhere
         *    mem.member_refs<std::vector<Sub *>>("sub_v_", {..});      // each printed elsewhere
         *    mem.end();                              // writes ]
         *
         *  See .xo-backlog/xo-websock/issues/13.
         **/
        class JsonMembers {
        public:
            /** writes on @p state's output; prints values through it **/
            explicit JsonMembers(JsonPrintState & state);
            /** asserts that end() was called -- unless a printer threw
             *  since this began
             **/
            ~JsonMembers();

            /** member @p name, holding @p value -- the member itself, an
             *  lvalue in the object, so it takes part in identity
             *  (JsonPrintState::print); its declared type is T
             **/
            template <typename T>
            JsonMembers & member(std::string_view name, T const & value) {
                return this->member_impl<T>(name, value, true /*identity*/);
            }

            /** member @p name, declared as type Declared, whose value is
             *  @p value -- computed, e.g. an atomic member read with
             *  load(), or a copy taken under a lock -- so with no identity
             *  (JsonPrintState::print_value)
             **/
            template <typename Declared, typename V>
            JsonMembers & member_as(std::string_view name, V const & value) {
                return this->member_impl<Declared>(name, value, false /*!identity*/);
            }

            /** member @p name, declared as type Declared, referring to an
             *  object printed in full elsewhere: _value_ is {"_ref_": n},
             *  or null (JsonPrintState::print_ref).  For an object owned
             *  elsewhere, or shared: it prints in full where it is owned.
             *  @p p must be the address that object prints at -- typed as
             *  its printer is keyed, not a dynamic_cast to void const *
             **/
            template <typename Declared>
            JsonMembers & member_ref(std::string_view name, void const * p) {
                this->write_ref(name, declared_of<Declared>(), p);

                return *this;
            }

            /** member @p name, declared as type Declared, a container of
             *  objects printed in full elsewhere: _value_ is an array, an
             *  element {"_ref_": n} or (a released slot) null -- so
             *  slot positions are kept
             **/
            template <typename Declared>
            JsonMembers & member_refs(std::string_view name, std::vector<void const *> const & ps) {
                this->write_refs(name, declared_of<Declared>(), ps);

                return *this;
            }

            /** member @p name, declared as type Declared, a map to objects
             *  printed in full elsewhere: _value_ is a json object, key ->
             *  {"_ref_": n} or null, keys in the order given (sort
             *  them, for an unordered container)
             **/
            template <typename Declared>
            JsonMembers & member_ref_map(std::string_view name,
                                         std::vector<std::pair<std::string, void const *>> const & kvs) {
                this->write_ref_map(name, declared_of<Declared>(), kvs);

                return *this;
            }

            /** close the array **/
            void end();

        private:
            /** a member's declared type: its names and metatype **/
            struct DeclaredType {
                std::string canonical_;
                std::string short_;
                reflect::Metatype metatype_;
            };

            /** declared type T, forwarding its TypeDescr.  xo-reflect has
             *  none for a C++ reference: its names are what a TypeDescr's
             *  would be (type_name<T>(), and make_short_name() of that),
             *  its metatype pointer, the nearest -- it refers to an object,
             *  rather than holding one
             **/
            template <typename T>
            static DeclaredType declared_of() {
                if constexpr (std::is_reference_v<T>) {
                    std::string canonical(reflect::type_name<T>());
                    std::string short_name = reflect::TypeDescrBase::make_short_name(canonical);
                    return DeclaredType{std::move(canonical), std::move(short_name),
                                        reflect::Metatype::mt_pointer};
                } else {
                    reflect::TypeDescr td = reflect::Reflect::require<T>();
                    return DeclaredType{td->canonical_name(), std::string(td->short_name()),
                                        td->metatype()};
                }
            }

            /** member @p name, declared Declared, holding @p value; with
             *  @p identity, @p value is a lasting lvalue
             **/
            template <typename Declared, typename V>
            JsonMembers & member_impl(std::string_view name, V const & value, bool identity) {
                using reflect::Reflect;
                using target_t = typename detail::member_target<V>::type;

                DeclaredType declared = declared_of<Declared>();
                reflect::TypeDescr target = Reflect::require<target_t>();

                if (this->printable(target)) {
                    this->write_value(name, declared,
                                      Reflect::make_tp(const_cast<V *>(&value)),
                                      identity);
                } else {
                    this->write_error(name, declared,
                                      "type not reflected: " + target->canonical_name());
                }

                return *this;
            }

            /** true iff PrintJson can print a @p td: it has a printer for
             *  it, or it is a complete reflected struct
             **/
            bool printable(reflect::TypeDescr td) const;

            void write_value(std::string_view name, DeclaredType const & declared,
                             reflect::TaggedPtr value, bool identity);
            void write_error(std::string_view name, DeclaredType const & declared,
                             std::string const & why);
            void write_ref(std::string_view name, DeclaredType const & declared,
                           void const * p);
            void write_refs(std::string_view name, DeclaredType const & declared,
                            std::vector<void const *> const & ps);
            void write_ref_map(std::string_view name, DeclaredType const & declared,
                               std::vector<std::pair<std::string, void const *>> const & kvs);
            /** {"_ref_": n}, or null **/
            void write_ref_value(void const * p);
            /** the separator and the entry's _name_, _canonical_type_,
             *  _short_type_, _metatype_
             **/
            void write_head(std::string_view name, DeclaredType const & declared);

        private:
            /** the print in progress **/
            JsonPrintState * state_ = nullptr;
            /** state_'s output **/
            std::ostream * p_os_ = nullptr;
            /** no entry written yet: no separator before the next **/
            bool first_ = true;
            bool ended_ = false;
            /** std::uncaught_exceptions() at construction: see ~JsonMembers **/
            int n_uncaught_ = 0;
        };
    } /*namespace json*/
} /*namespace xo*/

/* end JsonMembers.hpp */
