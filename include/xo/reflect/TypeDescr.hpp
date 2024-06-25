/* @file TypeDescr.hpp */

#pragma once

#include "TypeDescrExtra.hpp"
#include "xo/cxxutil/demangle.hpp"
#include <iostream>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cassert>

namespace xo {
    namespace reflect {
        class TaggedPtr;  /* see [reflect/TaggedPtr.hpp] */

        /* A reflected type is a type for which we keep information around at runtime
         * Assign reflected types unique (within an executable) ids,
         * allocating consecutively, starting from 1.
         * Reserve 0 as a sentinel
         */
        class TypeId {
        public:
            /* allocate a new TypeId value.
             * promise:
             * - retval.id() > 0
             */
            static TypeId allocate() { return TypeId(s_next_id++); }

            std::uint32_t id() const { return id_; }

        private:
            explicit TypeId(std::uint32_t id) : id_{id} {}

        private:
            static std::uint32_t s_next_id;

            /* unique index# for this type.
             * 0 reserved for sentinel
             */
            std::uint32_t id_ = 0;
        }; /*TypeId*/

        inline std::ostream &
        operator<<(std::ostream & os, TypeId x) {
            os << x.id();
            return os;
        } /*operator<<*/

        /* runtime description of a struct/class instance variable */
        class StructMember;

        class TypeDescrBase;

        using TypeDescr = TypeDescrBase const *;
        using TypeDescrW = TypeDescrBase *;

        /* convenience wrapper for a std::type_info pointer.
         * works properly with pybind11,  since python doens't encounter
         * native type_info pointer,  it won't try to delete it.
         */
        class TypeInfoRef {
        public:
            explicit TypeInfoRef(std::type_info const * tinfo) : tinfo_{tinfo} {}
            TypeInfoRef(TypeInfoRef const & x) = default;

            /* use:
             *   TypeInfoRef tinfo = TypeInfoRef::make<T>();
             */
            template<typename T>
            TypeInfoRef make() { return TypeInfoRef(&typeid(T)); }

            std::size_t hash_code() const { return this->tinfo_->hash_code(); }
            char const * impl_name() const { return this->tinfo_->name(); }

            static bool is_equal(TypeInfoRef x, TypeInfoRef y) noexcept {
                if (x.hash_code() != y.hash_code())
                    return false;

                return ::strcmp(x.impl_name(), y.impl_name()) == 0;
            } /*is_equal*/

        private:
            /* native type_info object for encapsulated type */
            std::type_info const * tinfo_ = nullptr;
        }; /*TypeInfoRef*/
    } /*namespace reflect*/
} /*namespace xo*/

namespace std {
    template <> struct hash<xo::reflect::TypeInfoRef> {
      std::size_t operator()(xo::reflect::TypeInfoRef x) const noexcept { return x.hash_code(); }
    };
} /*namespace std*/

namespace xo {
    namespace reflect {
        inline bool operator==(TypeInfoRef x, TypeInfoRef y) { return TypeInfoRef::is_equal(x, y); }
        inline bool operator!=(TypeInfoRef x, TypeInfoRef y) { return !TypeInfoRef::is_equal(x, y); }

#ifdef NOT_IN_USE
        namespace detail {
            class HashTypeInfoRef {
            public:
                std::size_t operator()(TypeInfoRef x) const noexcept { return x.hash_code(); }
            }; /*HashTypeInfoRef*/

            class EqualTypeInfoRef {
            public:
                bool operator()(TypeInfoRef x, TypeInfoRef y) const noexcept { return TypeInfoRef::is_equal(x, y); }
            }; /*EqualTypeInfoRef*/
        } /*namespace detail*/
#endif

        /* hashable contents of a FunctionTdx instance (without requiring decl of TypeDescrExtra),
         * for unique-ification of manually-constructed function types
         */
        struct FunctionTdxInfo {
            FunctionTdxInfo() = default;
            FunctionTdxInfo(TypeDescr retval_td,
                            const std::vector<TypeDescr> & arg_td_v,
                            bool is_noexcept)
                : retval_td_{retval_td},
                  arg_td_v_{arg_td_v},
                  is_noexcept_{is_noexcept}
                {}

            /** compare two FunctionTdxInfo objects for equality
             **/
            inline bool operator==(const FunctionTdxInfo & other) const noexcept {
                if (retval_td_ != other.retval_td_)
                    return true;
                if (arg_td_v_.size() != other.arg_td_v_.size())
                    return false;

                for (std::size_t i = 0, n = arg_td_v_.size(); i < n; ++i) {
                    if (arg_td_v_[i] != other.arg_td_v_[i])
                        return false;
                }

                if (is_noexcept_ != other.is_noexcept_)
                    return false;

                return true;
            }

            /** construct canonical description for this type
             *  will be like
             *    Retval(*)(Arg1,..,Argn)
             **/
            std::string make_canonical_name() const;

        public:
            /** function return value **/
            TypeDescr retval_td_ = nullptr;
            /** function arguments,  in positional order **/
            std::vector<TypeDescr> arg_td_v_;
            /** true iff function promises never to throw **/
            bool is_noexcept_ = false;
        }; /*FunctionTdxInfo*/

        class TypeDescrExtra;

        /* run-time description for a native c++ type */
        class TypeDescrBase {
        public:
            /* type-description objects for a type T is unique,
             *  --> can always use its address
             */
            TypeDescrBase(TypeDescrBase const & x) = delete;

            /* test whether a type has been reflected.
             * introducing this for unit testing
             */
            static bool is_reflected(std::type_info const * tinfo) {
                return (s_native_type_table_map.find(TypeInfoRef(tinfo))
                        != s_native_type_table_map.end());
            } /*is_reflected*/

            /* NOTE:
             *   implementation here will be defeated if std::type_info
             *   objects violate ODR.   This occurs with clang + 2-level namespaces,
             *   so important to linke with --flat_namespace defined.
             *   See FAQ
             *     [Build Issues|Q2 - dynamic_cast<Foo<*>> fails]
             */
            static TypeDescrW require(const std::type_info * tinfo,
                                      const std::string & canonical_name,
                                      std::unique_ptr<TypeDescrExtra> tdextra);

            /** Create type-description for function from input ingredients. **/
            static TypeDescrW require_by_fn_info(const FunctionTdxInfo & fn_info);

            /** lookup type by canonical name **/
            static TypeDescr lookup_by_name(const std::string & canonical_name);

            /** print table of reflected types to os **/
            static void print_reflected_types(std::ostream & os);
            /** print table of function types to os **/
            static void print_function_types(std::ostream & os);

            TypeId id() const { return id_; }
            const std::type_info * native_typeinfo() const { return native_typeinfo_; }
            const std::string & canonical_name() const { return canonical_name_; }
            const std::string_view & short_name() const { return short_name_; }
            bool complete_flag() const { return complete_flag_; }
            TypeDescrExtra * tdextra() const { return tdextra_.get(); }
            Metatype metatype() const { return tdextra_->metatype(); }

            /* true iff the type represented by *this is the same as the type
             * represented by T.
             *
             * Warning: comparing typeinfo address can give false negatives.
             *          suspect this is caused by problems coalescing linker symbols
             *          in the clang toolchain.
             */
            template<typename T>
            [[deprecated]]
            bool is_native() const {
                if (this->native_typeinfo()) {
                    /* reminder: typeid(T).name() is 'interesting' but not intended
                     *           to be human-readable.  It's not how compiler labels
                     *           a type for a human reader
                     */
                    return ((this->native_typeinfo() == &typeid(T))
                            || (this->native_typeinfo()->hash_code() == typeid(T).hash_code())
                            || (this->native_typeinfo()->name() == typeid(T).name()));
                } else {
                    /** if this type was established via Reflect::require<T1>(),
                     *  then .canonical_name is computed by type_name<T>()
                     *
                     *  (see demangle.hh in xo-refcnt,  which post-processes __PRETTY_FUNCTION__
                     *   or __FUNCSIG__)
                     *
                     *  To manually construct an equivalent type,
                     *  it's necessary to:
                     *  1. construct a unique and unambiguous canonical name for the type
                     *  2. be aware that type will only be recognized as equivalent to
                     *     a natively-reflected type if canonical name matches exactly.
                     **/

                    /** FOR NOW: give up. **/
                    throw std::runtime_error("TypeDescrBase::is_native: not implemented for manually-constructed TypeDescr objects.  Prefer is_native2()");
                }
            } /*is_native*/

            /** safe downcast -- like dynamic_cast<>,  but does not require a source type.
             *
             *  TODO: need variation on this to correctly-handle function types,
             *        since for exampple cast from void* -> void (*)() is not allowed
             *
             *  WARNING: relies on deprecated is_native<T>().  Application code should prefer any of:
             *  1. recover_native2(src_td, src_address)
             *  2. Reflect::recover_native<T>(src_td, src_address)
             *  3. TaggedPtr(src_td,src_address).recover_native<T>()
             *  instead of src_td->recover_native<T>()
             *
             *  (note: awkwardness here is that we don't have access to {Reflect.hpp, TaggedPtr.hpp}
             *   from this .hpp file,  since TypeDescr.hpp is included by those headers)
             **/
            template<typename T>
            [[deprecated]]
            T * recover_native(void * address) const {
                if (this->is_native<T>()) {
                    return reinterpret_cast<T *>(address);
                } else {
                    return nullptr;
                }
            } /*recover_native*/

            /** safe downcast -- like dynamic_cast<>,  but does not require a source type.
             *
             *  Application code should prefer TaggedPtr::recover_native<T>()
             *
             *  TODO: need variation on this to correctly-handle function types,
             *        since for exampple cast from void* -> void (*)() is not allowed
             **/
            template<typename T>
            T * recover_native2(TypeDescr address_td, void * address) const {
                if (this == address_td) {
                    return reinterpret_cast<T *>(address);
                } else {
                    return nullptr;
                }
            } /*recover_native2*/

            bool is_pointer() const { return this->tdextra_->is_pointer(); }
            bool is_vector() const { return this->tdextra_->is_vector(); }
            bool is_struct() const { return this->tdextra_->is_struct(); }
            bool is_function() const { return this->tdextra_->is_function(); }

            /* given a T-instance object,  return tagged pointer with T replaced
             * by the most-derived-subtype of T to which *object belongs.
             * This works only for descendants of reflect::SelfTagging
             */
            TaggedPtr most_derived_self_tp(void * object) const;

            /* if generalized vector (std::vector<T>, std::array<T,N>, ..):
             *   .n_child() reports #of elements
             * if struct/class:
             *   .n_child() reports #of instance variables (that have been reflected)
             */
            uint32_t n_child(void * object) const { return this->tdextra_->n_child(object); }
            TaggedPtr child_tp(uint32_t i, void * object) const;

            /* require:
             * - .is_struct() = true
             * - i in [0 .. .n_child() - 1]
             */
            std::string const & struct_member_name(uint32_t i) const {
                return this->tdextra_->struct_member_name(i);
            }

            /* fetch runtime description for i'th reflected instance variable.
             *
             * require:
             * - .is_struct() = true
             * - i in [0 .. .n_child() - 1]
             */
            StructMember const & struct_member(uint32_t i) const {
                StructMember const * sm = this->tdextra_->struct_member(i);

                assert(sm);
                return *sm;
            } /*struct_member*/

            /** nullptr for non-function types **/
            const FunctionTdxInfo * fn_info() const { return this->tdextra_->fn_info(); }
            uint32_t n_fn_arg() const { return this->tdextra_->n_fn_arg(); }

            /* require:
             * - .is_function() = true
             */
            TypeDescr fn_retval() const { return this->tdextra_->fn_retval(); }
            TypeDescr fn_arg(uint32_t i) const { return this->tdextra_->fn_arg(i); }
            bool fn_is_noexcept() const { return this->tdextra_->fn_is_noexcept(); }

            void display(std::ostream & os) const;
            std::string display_string() const;

            /* mark this TypeDescr complete;
             * returns the value of .complete_flag from _before_
             * this call
             */
            bool mark_complete();

            /* call this once to attach extended type information to a type-description
             * (e.g. description of struct members for a record type)
             */
            void assign_tdextra(std::unique_ptr<TypeDescrExtra> tdx) {
                this->complete_flag_ = true;
                this->tdextra_ = std::move(tdx);
            }

        private:
            TypeDescrBase(TypeId id,
                          const std::type_info * tinfo,
                          const std::string & canonical_name,
                          std::unique_ptr<TypeDescrExtra> tdextra);

            void assign_native_tinfo(const std::type_info * tinfo) {
                assert(!native_typeinfo_);
                native_typeinfo_ = tinfo;
            }

        private:
            /* invariant:
             * - for all TypeDescrImpl instances x:
             *   - s_type_table_v[x->id()] = x
             *   - s_native_type_table_map[TypeInfoRef(x->typeinfo())] = x
             */

            /** vector of all TypeDescr instances, indexed by TypeId.  singleton. **/
            static std::vector<std::unique_ptr<TypeDescrBase>> s_type_table_v;

            /** hashmap of all TypeDescr instances,
             *  indexed by canonical_name.
             *
             *  For manually-constructed TypeDescr instances
             *  (see xo-expression for use-case) we require:
             *
             *  - TypeDescr::canonical_name uniquely identifies type
             *  - to interact with an actually-equivalent type T
             *    constructed by c++ compiler,  we need
             *    to use the same canonical name that the compiler uses.
             *
             *    See type xo::reflect::type_name<>() [in demangle.hpp under xo-refcnt]
             *    for implementation
             **/
            static std::unordered_map<std::string, TypeDescrBase*> s_canonical_type_table_map;

            /** hashmap of all native TypeDescr instances,
             *  indexed by typeinfo. singleton.
             **/
            static std::unordered_map<TypeInfoRef, TypeDescrBase *> s_native_type_table_map;

            /** hashmap of (presumed) duplicate TypeInfoRef values.
             *  This happens with clang sometimes when the same type is referenced
             *  from multiple modules (i.e. shared libs).
             **/
            static std::unordered_map<TypeInfoRef, TypeDescrBase *> s_coalesced_type_table_map;

            /** map from a vector of TypeDescr objects:
             *    [Retval, Arg1, ...Argn]
             *  to TypeDescr for function type
             *    Retval(*)(Arg1..Argn)
             *
             *  Use these to unique-ify function types across:
             *  - types sourced natively from c++ compiler
             *  - types manually constructed (e.g. see Lambda.cpp in xo-expression)
             **/
            static std::unordered_map<FunctionTdxInfo, TypeDescrBase *> s_function_type_map;

        private:
            /* unique id# for this type */
            TypeId id_;
            /** typeinfo for type T,  if available.  nullptr otherwise.
             *
             *  1. Always available for type-descriptions constructed via Reflect::require<T>.
             *  2. Always missing for manually-constructed TypeDescr instances, for example
             *     see Lambda.cpp in xo-expression.
             **/
            std::type_info const * native_typeinfo_ = nullptr;
            /** canonical name for this type (see demangle.hpp for type_name<T>())
             * e.g.
             *   xo::option::Px2
             *
             * NOTE: if we only had to deal with types created via Reflect::reflect<T>(),
             *       then canonical_name could be string_view.  For manually-constructed
             *       types,  there is no compiler-generated C-string constant to reference,
             *       so need to use std::string here
             **/
            std::string canonical_name_;
            /** substring .canonical_name, just after last ':'
             *  e.g.
             *    Px2
             **/
            std::string_view short_name_;
            /* set to true once final value for .tdextra is established
             * intially all TypeDescr objects will use AtomicTdx for .tdextra
             * Reflect::require() upgrades .tdextra for particular types.
             * When that procedure makes a decision for a type T,
             * .complete_flag will be set to true for the corresponding TypeDescrBase instance
             */
            bool complete_flag_ = false;
            /* additional type information that either:
             * (a) isn't universal across all types,
             *     e.g. dereferencing instance of a pointer type
             * (b) can't be captured with template-fu,
             *     e.g. struct member names
             *
             * generally .tdextra will be populated some time after TypeDescrBase's ctor exits.
             * This is necessary because of (b) above,  also because of possibility of recursive
             * types.
             */
            std::unique_ptr<TypeDescrExtra> tdextra_;
        }; /*TypeDescrBase*/

        inline std::ostream &
        operator<<(std::ostream & os, TypeDescrBase const & x) {
            x.display(os);
            return os;
        } /*operator<<*/

        /* tag to drive overload resolution */
        struct reflected_types_printer {};

        inline std::ostream &
        operator<<(std::ostream & os, reflected_types_printer) {
            TypeDescrBase::print_reflected_types(os);
            return os;
        }

        class TypeDescrTable {
        public:
            TypeDescrTable * instance() { return &s_instance; }

        private:
            /** initialize with builtin atomic types:
             *    float, double, char, short, int, long, bool
             **/
            TypeDescrTable();

        private:
            static TypeDescrTable s_instance;
        };
    } /*namespace reflect*/
} /*namespace xo*/

namespace std {
    /** @brief overload for hashing xo::reflect::FunctionTdxInfo objects
     **/
    template <>
    struct hash<xo::reflect::FunctionTdxInfo> {
        inline size_t operator()(const xo::reflect::FunctionTdxInfo & x) const noexcept {
            /* we can hash on addresses,  since TypeDescr objects are immutable */
            std::size_t h = hash<xo::reflect::TypeDescr>{}(x.retval_td_);

            for (std::size_t i = 0, n = x.arg_td_v_.size(); i < n; ++i) {
                h = (h << 1) ^ hash<xo::reflect::TypeDescr>{}(x.arg_td_v_[i]);
            }

            h = (h << 1) ^ (x.is_noexcept_ ? 1 : 0);

            return h;
        }
    };
}

/* end TypeDescr.hpp */
