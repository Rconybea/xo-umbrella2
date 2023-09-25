/* @file TypeDescr.hpp */

#pragma once

//#include "reflect/atomic/AtomicTdx.hpp"
#include "reflect/TypeDescrExtra.hpp"
#include "cxxutil/demangle.hpp"
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
    return (s_type_table_map.find(TypeInfoRef(tinfo))
        != s_type_table_map.end());
      } /*is_reflected*/

      /* NOTE:
       *   implementation here will be defeated if std::type_info
       *   objects violate ODR.   This occurs with clang + 2-level namespaces,
       *   so important to linke with --flat_namespace defined.
       *   See FAQ
       *     [Build Issues|Q2 - dynamic_cast<Foo<*>> fails]
       */
      static TypeDescrW require(std::type_info const * tinfo,
                std::string_view canonical_name,
                std::unique_ptr<TypeDescrExtra> tdextra);

      /* print table of reflected types to os */
      static void print_reflected_types(std::ostream & os);

      TypeId id() const { return id_; }
      std::type_info const * typeinfo() const { return typeinfo_; }
      std::string_view const & canonical_name() const { return canonical_name_; }
      std::string_view const & short_name() const { return short_name_; }
      bool complete_flag() const { return complete_flag_; }
      TypeDescrExtra * tdextra() const { return tdextra_.get(); }
      Metatype metatype() const { return tdextra_->metatype(); }

      /* true iff the type represented by *this is the same as the type T.
       *
       * Warning: comparing typeinfo address can give false negatives.
       *          suspect this is caused by problems coalescing linker symbols
       *          in the clang toolchain.
       */
      template<typename T>
      bool is_native() const {
    return ((this->typeinfo() == &typeid(T))
        || (this->typeinfo()->hash_code() == typeid(T).hash_code())
        || (this->typeinfo()->name() == typeid(T).name()));
      } /*is_native*/

      /* safe downcast -- like dynamic_cast<>,  but does not require a source type */
      template<typename T>
      T * recover_native(void * address) const {
    if (this->is_native<T>()) {
      return reinterpret_cast<T *>(address);
    } else {
      return nullptr;
    }
      } /*recover_native*/

      bool is_vector() const { return this->tdextra_->is_vector(); }
      bool is_struct() const { return this->tdextra_->is_struct(); }

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
      void assign_tdextra(std::unique_ptr<TypeDescrExtra> tdx);

    private:
      TypeDescrBase(TypeId id,
            std::type_info const * tinfo,
            std::string_view canonical_name,
            std::unique_ptr<TypeDescrExtra> tdextra);

    private:
      /* invariant:
       * - for all TypeDescrImpl instances x:
       *   - s_type_table_v[x->id()] = x
       *   - s_type_table_map[TypeInfoRef(x->typeinfo())] = x
       */

      /* hashmap of all TypeDescr instances, indexed by .  singleton */
      static std::unordered_map<TypeInfoRef, std::unique_ptr<TypeDescrBase>> s_type_table_map;
      /* hashmap of (presumed) duplicate TypeInfoRef values.
       * This happens with clang sometimes when the same type is referenced
       * from multiple modules (i.e. shared libs).
       */
      static std::unordered_map<TypeInfoRef, TypeDescrBase *> s_coalesced_type_table_map;

      /* vector of all TypeDescr instances.  singleton. */
      static std::vector<TypeDescrBase *> s_type_table_v;

    private:
      /* unique id# for this type */
      TypeId id_;
      /* typeinfo for type T */
      std::type_info const * typeinfo_ = nullptr;
      /* canonical name for this type (see demangle.hpp for type_name<T>())
       * e.g.
       *   xo::option::Px2
       */
      std::string_view canonical_name_;
      /* suffix of .canonical_name, just after last ':'
       * e.g.
       *   Px2
       */
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

  } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescr.hpp */
