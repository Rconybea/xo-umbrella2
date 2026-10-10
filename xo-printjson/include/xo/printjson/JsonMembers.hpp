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

            /** write one entry per reflected member of a struct
             * described by @p obj.
             * No-op if @p obj does not refer to a reflected struct.
             *
             * When reporting a member name, suffix with @p name_suffix.
             *
             * reflected_member() safely traverses members that require
             * locking, provided those relationships have been reflected.
             * It will acquire a lock L once for each set of members that L guards.
             *
             * If mode is @c GuardMode::try_lock, and reflected_members()
             * encounters a lock that is already held, it will report
             * @code
             *   {"_name_": .., .., "_locked_": true}
             * @endcode
             * instead of attempting to traverse the locked member.
             **/
            JsonMembers & reflected_members(reflect::TaggedPtr obj,
                                            std::string_view name_suffix = {});

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
                reflect::TaggedPtr tp = Reflect::make_tp(const_cast<V *>(&value));

                /* by the VALUE, as reflected_members() decides: a null
                 * pointer, an empty vector, a ref to an object printed
                 * already -- each printable, whatever the target's type
                 */
                if (this->printable_value(tp)) {
                    this->write_value(name, declared, tp, identity);
                } else {
                    this->write_error(name, declared,
                                      "type not reflected: " + target->canonical_name());
                }

                return *this;
            }

            /** true iff PrintJson can print a @p td: it has a printer for
             *  it, or it is a complete reflected struct, or a reflected enum,
             *  or a std::atomic<T> of a printable T, or a transparent wrapper
             *  of a printable value
             **/
            bool printable(reflect::TypeDescr td) const;
            /** printable(), for a value: through a pointer to its target,
             *  through a vector to its first element (null and empty are
             *  printable)
             **/
            bool printable_value(reflect::TaggedPtr v) const;

            /** write @p value of a member @p name declared with type @p declared.
             *  @p identity is true for a value that exists outside
             *  a PrintJson excursion; false for a temporary
             *  (e.g. result of loading a @c std::atomic during traverse).
             *  Will try to traverse a non-leaf @p value.
             **/
            void write_value(std::string_view name, DeclaredType const & declared,
                             reflect::TaggedPtr value, bool identity);
            /** as write_value, for a pointer @p ptr whose pointee is reached
             *  by an edge of kind @p edge (JsonPrintState::print_pointee)
             **/
            void write_pointee(std::string_view name, DeclaredType const & declared,
                               reflect::TaggedPtr ptr, reflect::Ownership edge);
            void write_error(std::string_view name, DeclaredType const & declared,
                             std::string const & why);
            /** member @p name is inaccessible because guard is busy; report as locked **/
            void write_locked(std::string_view name, DeclaredType const & declared);
            /** member @p i of reflected struct @p obj, read now **/
            void write_reflected(reflect::TaggedPtr obj, std::uint32_t i,
                                 std::string_view name_suffix, bool readable);
            void write_ref(std::string_view name, DeclaredType const & declared,
                           void const * p);
            void write_refs(std::string_view name, DeclaredType const & declared,
                            std::vector<void const *> const & ps);
            void write_ref_map(std::string_view name, DeclaredType const & declared,
                               std::vector<std::pair<std::string, void const *>> const & kvs);
            /** write
             *  @code {"_ref_": n}
             *  @endcode
             *  or null
             **/
            void write_ref_value(void const * p);
            /** write separator and the entry's @c _name_, @c _canonical_type_,
             *  @c _short_type_, @c _metatype_ attributes
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
