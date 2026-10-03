/** @file JsonMembers.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "PrintJson.hpp"
#include <xo/reflect/Reflect.hpp>
#include <xo/reflectutil/type_name.hpp>
#include <xo/refcnt/Refcounted.hpp>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
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

        /** an object's identity in json output: its address, as a string.
         *  An object printed in full writes it as its "id"; a reference to it,
         *  {"ref": json_id(p)}, so a consumer can join the two.  Unique within
         *  one output; an address may be reused once its object is freed
         **/
        std::string json_id(void const * p);

        /** @brief writes an object's "_members_" array, for a JsonPrinter that
         *  opts in to showing chosen C++ members:
         *
         *    "_members_": [{"_name_": .., "_type_": .., "_metatype_": .., "_value_": ..}, ..]
         *
         *  _type_ is the member's DECLARED type (its canonical name);
         *  _metatype_ that type's xo-reflect metatype (atomic, pointer,
         *  vector, struct, function -- metatype2str); _value_
         *  is printed by PrintJson as any value would be -- so an rp<T> or T*
         *  reflects as its pointee's actual type, and a nested object carries
         *  its own _members_ if its printer writes them.
         *
         *  A member whose type cannot be printed -- neither a json printer
         *  nor a complete reflected struct -- is written without a value:
         *    {"_name_": .., "_type_": .., "_metatype_": .., "_error_": "type not reflected: X"}
         *  so one omission does not spoil the rest of the output.
         *
         *  Use, inside a JsonPrinter's print_json, after the printer's own
         *  keys:
         *
         *    JsonMembers mem(this->pjson(), p_os);   // writes , "_members_": [
         *    mem.member("url_router_", x->url_router_);
         *    mem.member_as<std::atomic<int>>("port_", x->port_.load());
         *    mem.member_ref<rp<Sender>>("sender_", x->sender_.get());  // printed elsewhere
         *    mem.end();                              // writes ]
         *
         *  See .xo-backlog/xo-websock/issues/13.
         **/
        class JsonMembers {
        public:
            JsonMembers(PrintJson const * pjson, std::ostream * p_os);
            /** asserts that end() was called **/
            ~JsonMembers();

            /** member @p name, holding @p value; its declared type is T **/
            template <typename T>
            JsonMembers & member(std::string_view name, T const & value) {
                return this->member_as<T>(name, value);
            }

            /** member @p name, declared as type Declared, whose value is
             *  @p value -- e.g. an atomic member read with load()
             **/
            template <typename Declared, typename V>
            JsonMembers & member_as(std::string_view name, V const & value) {
                using reflect::Reflect;
                using target_t = typename detail::member_target<V>::type;

                std::string declared(reflect::type_name<Declared>());
                reflect::Metatype metatype = Reflect::require<Declared>()->metatype();
                reflect::TypeDescr target = Reflect::require<target_t>();

                if (this->printable(target)) {
                    this->write_value(name, declared, metatype,
                                      Reflect::make_tp(const_cast<V *>(&value)));
                } else {
                    this->write_error(name, declared, metatype,
                                      "type not reflected: " + target->canonical_name());
                }

                return *this;
            }

            /** member @p name, declared as type Declared, referring to an
             *  object printed in full elsewhere: _value_ is {"ref":
             *  json_id(@p p)}, or null.  For an object owned elsewhere, or
             *  shared -- printing it in full here would repeat it, or recurse
             **/
            template <typename Declared>
            JsonMembers & member_ref(std::string_view name, void const * p) {
                using reflect::Reflect;

                this->write_ref(name, std::string(reflect::type_name<Declared>()),
                                Reflect::require<Declared>()->metatype(), p);

                return *this;
            }

            /** close the array **/
            void end();

        private:
            /** true iff PrintJson can print a @p td: it has a printer for
             *  it, or it is a complete reflected struct
             **/
            bool printable(reflect::TypeDescr td) const;

            void write_value(std::string_view name, std::string const & declared,
                             reflect::Metatype metatype, reflect::TaggedPtr value);
            void write_error(std::string_view name, std::string const & declared,
                             reflect::Metatype metatype, std::string const & why);
            void write_ref(std::string_view name, std::string const & declared,
                           reflect::Metatype metatype, void const * p);
            /** the separator and the entry's _name_, _type_, _metatype_ **/
            void write_head(std::string_view name, std::string const & declared,
                            reflect::Metatype metatype);

        private:
            PrintJson const * pjson_ = nullptr;
            std::ostream * p_os_ = nullptr;
            /** no entry written yet: no separator before the next **/
            bool first_ = true;
            bool ended_ = false;
        };
    } /*namespace json*/
} /*namespace xo*/

/* end JsonMembers.hpp */
