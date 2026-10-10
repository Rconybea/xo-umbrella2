/** @file JsonObject.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include "JsonPrintState.hpp"
#include "JsonMembers.hpp"
#include <xo/reflect/Reflect.hpp>
#include <exception>
#include <string_view>

namespace xo {
    namespace json {
        /** @brief writes one json object, for a JsonPrinter
         *
         *  From JsonPrintState::open_object, which has written
         *    {"_name_": .., "_canonical_type_": .., "_short_type_": .., "_id_": n
         *  Add keys, then any "_members_", then close():
         *
         *    JsonObject obj = state.open_object(tp);
         *    obj.key("refcount", x->reference_counter())    // a computed value
         *       .child("config", Reflect::make_tp(&x->config_))   // part of x
         *       .key_ref("sender", x->sender_.get());        // printed elsewhere
         *    obj.members()
         *       .member("port_", x->port_)
         *       .end();
         *    obj.close();
         **/
        class JsonObject {
        public:
            using Reflect = xo::reflect::Reflect;
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            JsonObject(JsonObject const &) = delete;
            JsonObject & operator=(JsonObject const &) = delete;
            /** takes over @p x's object: @p x no longer needs closing **/
            JsonObject(JsonObject && x)
                : state_{x.state_}, closed_{x.closed_}, is_root_{x.is_root_},
                  n_uncaught_{x.n_uncaught_} { x.closed_ = true; }
            /** asserts close() was called -- unless a printer threw
             *  since this object opened
             **/
            ~JsonObject();

            /** , "@p k": @p tp -- part of this object (a member, say), so
             *  it takes part in identity (JsonPrintState::print)
             **/
            JsonObject & child(std::string_view k, TaggedPtr tp);

            /** , "@p k": @p v -- a computed value, with no identity
             *  (JsonPrintState::print_value)
             **/
            template <typename T>
            JsonObject & key(std::string_view k, T const & v) {
                this->write_key(k);
                state_->print_value(Reflect::make_tp(const_cast<T *>(&v)));
                return *this;
            }

            /** , "@p k": {"_ref_": n} or null -- an object printed in full
             *  elsewhere (JsonPrintState::print_ref)
             **/
            JsonObject & key_ref(std::string_view k, void const * p);

            /** , "@p k": -- and the stream, for a value the printer writes
             *  itself (an array, say), printing any objects in it through
             *  state()
             **/
            std::ostream & key_open(std::string_view k);

            /** , "_members_": [ .. ] -- after any keys **/
            JsonMembers members();

            /** writes } -- after "_unplaced_", if this is the top-level
             *  value's object and any are (JsonPrintState)
             **/
            void close();

            /** the print in progress **/
            JsonPrintState & state() const { return *state_; }

        private:
            friend class JsonPrintState;

            JsonObject(JsonPrintState * state, bool is_root)
                : state_{state}, is_root_{is_root}, n_uncaught_{std::uncaught_exceptions()} {}

            /** , "@p k": **/
            void write_key(std::string_view k);

        private:
            /** the print in progress **/
            JsonPrintState * state_ = nullptr;
            bool closed_ = false;
            /** the top-level value's own object: writes "_unplaced_" **/
            bool is_root_ = false;
            /** std::uncaught_exceptions() when opened: more at destruction
             *  means unwinding, where close() cannot have run
             **/
            int n_uncaught_ = 0;
        }; /*JsonObject*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonObject.hpp */
