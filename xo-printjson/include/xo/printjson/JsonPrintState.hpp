/** @file JsonPrintState.hpp
 *
 *  @author Roland Conybeare, Oct 2026
 **/

#pragma once

#include <xo/reflect/TaggedPtr.hpp>
#include <xo/reflect/TypeDescr.hpp>
#include <cstdint>
#include <iosfwd>
#include <string_view>
#include <unordered_map>

namespace xo {
    namespace json {
        class PrintJson;
        class JsonObject;

        /** @brief one json print in progress
         *
         *  PrintJson's entry points (print, print_tp, print_obj) make one
         *  per top-level value.  A JsonPrinter receives it, and writes and
         *  recurses only through it: what a print needs to know about
         *  itself travels here, in plain sight, rather than in the printer
         *  or the printer table.
         *
         *  A printer must not call an entry point to print a child: that
         *  starts a print within a print.  See
         *  .xo-backlog/xo-printjson/issues/02.
         *
         *  Each object prints once.  The first time a print reaches an
         *  object -- a value its printer writes as a json object -- it
         *  prints in full, carrying "_id_": n; every later time, as
         *  {"_ref_": n}.  So any graph, cycles included, prints in output
         *  linear in its size.  Ids number objects 1, 2, 3 .. in order of
         *  first mention, within one print.
         *
         *  An object's identity is its address, and the type it first
         *  printed as.  Another object at the same address -- a struct's
         *  first by-value member -- is part of the first: it prints in
         *  full, with no "_id_", and is never a ref's target.
         *
         *  Nesting is bounded: print() aborts, with a backtrace, past
         *  PrintJson::max_depth() -- see print()
         **/
        class JsonPrintState {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;
            using TypeDescr = xo::reflect::TypeDescr;

        public:
            /** a print of json on @p p_os, through @p pjson's printers,
             *  nesting at most @p pjson->max_depth() deep
             **/
            JsonPrintState(PrintJson const * pjson, std::ostream * p_os);

            JsonPrintState(JsonPrintState const &) = delete;
            JsonPrintState & operator=(JsonPrintState const &) = delete;

            /** where json goes **/
            std::ostream * p_os() const { return p_os_; }

            /** true iff a printer was provided for @p td (PrintJson::provide_printer) **/
            bool has_printer(TypeDescr td) const;

            /** write json for @p tp: by its type's printer, if one was
             *  provided, else by the generic behaviour for its metatype
             *  (pointer, vector, struct).  The one way a printer recurses.
             *
             *  @p tp is a lasting object -- a member, an element, a
             *  pointee -- so takes part in identity: an object printed
             *  already becomes {"_ref_": n}.
             *
             *  Each call nests one deeper than its caller.  A call that
             *  would nest deeper than max_depth() prints a diagnosis and a
             *  backtrace on stderr, then aborts
             **/
            void print(TaggedPtr tp);

            /** as print(), for a computed value with no lasting address
             *  (e.g. an atomic's load()): no identity for @p tp itself,
             *  since another temporary may later occupy its address.  What
             *  it points to still takes part
             **/
            void print_value(TaggedPtr tp);

            /** {"_ref_": n} for the object at @p p, printed in full elsewhere
             *  in this print (before or after), or null.  @p p must be the
             *  address that object prints at: typed as its printer is keyed
             *  (e.g. a WsSender const *, not a dynamic_cast to void const *)
             **/
            void print_ref(void const * p);

            /** open the json object for the value being printed, from its
             *  printer's print_json; at most once per print_json call.
             *  Writes {"_name_": @p name, the type keys of @p td, and (if
             *  it has identity) "_id_": n.  Add keys, then close()
             **/
            JsonObject open_object(std::string_view name, TypeDescr td);

            /** open_object, named and typed by @p tp's type **/
            JsonObject open_object(TaggedPtr tp);

            /** true iff the object at @p p has printed in full, in this print **/
            bool is_printed(void const * p) const;

            /** open a json object for the object at @p p, one the printer
             *  writes inline itself rather than through print(): it takes
             *  part in identity like any other, "_id_" included.  Ask
             *  is_printed(@p p) first -- if so, print_ref(@p p) instead.
             *  Independent of the value print_json was called for
             **/
            JsonObject open_object_at(void const * p, std::string_view name, TypeDescr td);

            /** current nesting: print() calls in progress **/
            std::uint32_t depth() const { return depth_; }

            /** nesting limit, from PrintJson::max_depth() **/
            std::uint32_t max_depth() const { return max_depth_; }

        private:
            /** an object met in this print **/
            struct ObjectEntry {
                /** its "_id_" / "_ref_" **/
                std::uint32_t id_ = 0;
                /** the type it printed as; nullptr while only referred to **/
                TypeDescr type_ = nullptr;
            };

            /** the value print_node is dispatching, for open_object **/
            struct Pending {
                /** a print_json call is in progress that has not yet opened its object **/
                bool open_ = false;
                /** the value's address, if it takes part in identity; else nullptr **/
                void const * address_ = nullptr;
                /** the value's type **/
                TypeDescr type_ = nullptr;
            };

            /** print(), print_value(): @p identity false for print_value **/
            void print_node(TaggedPtr tp, bool identity);
            /** the entry for the object at @p p, made (unprinted) on first mention **/
            ObjectEntry & entry_for(void const * p);
            /** past max_depth_: diagnose, backtrace, abort **/
            [[noreturn]] void abort_too_deep(TaggedPtr tp) const;
            /** printer misuse (open_object twice, or outside print_json): diagnose, abort **/
            [[noreturn]] void abort_misuse(char const * what) const;
            /** {"_name_": .., type keys, and "_id_" if @p p non-null: owns @p p's entry **/
            JsonObject open_object_aux(void const * p, std::string_view name, TypeDescr td);

        private:
            /** the printer table **/
            PrintJson const * pjson_ = nullptr;
            /** where json goes **/
            std::ostream * p_os_ = nullptr;
            /** print() calls in progress **/
            std::uint32_t depth_ = 0;
            /** print() aborts past this depth **/
            std::uint32_t max_depth_ = 0;
            /** objects met in this print, by address.  A std::unordered_map
             *  for now; a DArenaHashMap from a pool of temporary arenas is
             *  the intent (.xo-backlog/xo-printjson/issues/02)
             **/
            std::unordered_map<void const *, ObjectEntry> objects_;
            /** id for the next object first mentioned **/
            std::uint32_t next_id_ = 1;
            /** see Pending **/
            Pending pending_;
        }; /*JsonPrintState*/

    } /*namespace json*/
} /*namespace xo*/

/* end JsonPrintState.hpp */
