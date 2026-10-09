/* @file StructMember.hpp */

#pragma once

#include "xo/reflect/EstablishTypeDescr.hpp"
#include "xo/reflect/Ownership.hpp"
#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/TypeDescr.hpp"
#include <cassert>
#include <memory>
#include <optional>
#include <string>

namespace xo {
    namespace reflect {
        class AbstractStructMemberAccessor {
        public:
            virtual ~AbstractStructMemberAccessor() = default;

            /* get tagged pointer referring to this member of the object at *struct_addr */
            TaggedPtr member_tp(void * struct_addr) const;

            /* get type-description object for struct
             * containing this member.  useful for consistency checking.
             */
            virtual TypeDescr struct_td() const = 0;

            /* get type-description object for this member
             * e.g. if this member represents Foo::bar_ in
             *   struct Foo { int bar_; };
             * then
             *   .member_td() => Reflect::require<int>();
             */
            virtual TypeDescr member_td() const = 0;

            /* get address of a particular member,  given parent address */
            virtual void * address(void * struct_addr) const = 0;

            virtual std::unique_ptr<AbstractStructMemberAccessor> clone() const = 0;

            /* true iff @p x reaches the same member, by the same route:
             * how StructReflector interns guards (StructMemberDecl::guarded_by)
             */
            virtual bool same_member(AbstractStructMemberAccessor const & x) const = 0;
        }; /*AbstractStructMemberAccessor*/

        /* GeneralStructMemberAccessor
         *
         * Use this to handle access to possibly-inherited struct members:
         *
         *   struct Foo { int x_; }
         *   struct Bar { char * y_; }
         *   struct Quux : public Foo, public Bar { bool z_; }
         *
         * want to be able to access Bar::y from a Quux instance.
         * in example,  would use GenericStructMemberAccessor<>
         * with:
         *   StructT = Quux,
         *   OwnerT = Bar,
         *   MemberT = char*
         *
         * Require:
         *   StructT* is assignable to OwnerT* (because StructT --isa--> OwnerT)
         */
        template <typename StructT, typename OwnerT, typename MemberT>
        class GeneralStructMemberAccessor : public AbstractStructMemberAccessor {
        public:
            /* pointer to a OwnerT member of type MemberT */
            using Memptr = MemberT OwnerT::*;

        public:
            GeneralStructMemberAccessor(Memptr memptr)
                : member_td_{EstablishTypeDescr::establish<MemberT>()},
                  memptr_{memptr} {}
            GeneralStructMemberAccessor(GeneralStructMemberAccessor const & x) = default;
            virtual ~GeneralStructMemberAccessor() = default;

            static std::unique_ptr<GeneralStructMemberAccessor> make(Memptr memptr) {
                return std::unique_ptr<GeneralStructMemberAccessor>(new GeneralStructMemberAccessor(memptr)); }

            /* get member address given address of parent struct
             * (i.e. from Struct*,  not from OwnerT*)
             */
            MemberT * address_impl(StructT * self_addr) const {
                OwnerT * owner_addr = self_addr;

                return &(owner_addr->*memptr_);
            } /*address_impl*/

            // ----- Inherited from AbstractStructMemberAccessor -----

#ifdef OBSOLETE
            virtual TaggedPtr member_tp(void * struct_addr) const override {
                /* FIXME: this reports declared type of member,  instead of
                 *        (possibly narrower) actual type of member
                 */

                return this->member_td_->most_derived_self_tp(this->address(struct_addr));
                //return TaggedPtr(this->member_td_, this->address(struct_addr));
            } /*member_tp*/
#endif

            virtual TypeDescr struct_td() const override { return EstablishTypeDescr::establish<StructT>(); }

            virtual TypeDescr member_td() const override { return this->member_td_; }

            virtual void * address(void * struct_addr) const override {
                return this->address_impl(reinterpret_cast<StructT *>(struct_addr));
            } /*address*/

            virtual std::unique_ptr<AbstractStructMemberAccessor> clone() const override {
                return std::unique_ptr<AbstractStructMemberAccessor>
                    (new GeneralStructMemberAccessor(*this));
            } /*clone*/

            virtual bool same_member(AbstractStructMemberAccessor const & x) const override {
                auto const * other = dynamic_cast<GeneralStructMemberAccessor const *>(&x);

                return other && (other->memptr_ == this->memptr_);
            } /*same_member*/

        private:
            /* type description for MemberT; .memptr is pointer-to-member-of-OwnerT,
             * where that member has type MemberT
             */
            TypeDescr member_td_ = nullptr;
            /* pointer to member of OwnerT */
            Memptr memptr_ = nullptr;
        }; /*GeneralStructMemberAccessor*/

        /* struct-member accessor via delegation,
         * to accessor of a parent (or some other ancestor) class.
         *
         *   struct Foo { int x_; }
         *   struct Bar { char * y_; }
         *
         *   auto bar_x_access = GeneralStructMemberAccessor<Bar, Foo, int>::make(&Foo::x_);
         *
         * or equivalently:
         *   auto foo_x_access = GeneralStructMemberAccessor<Foo, Foo, int>::make(&Foo::x_);
         *   auto bar_x_access = AncestorStructMemberAccessor<Bar, Foo>::adopt(foo_x_access);
         *
         * can use the 2nd form to adopt accessors from an already-reflected ancestor class
         *
         * Require:
         * - StructT -isa-> AncestorT
         */
        template <typename StructT, typename AncestorT>
        class AncestorStructMemberAccessor : public AbstractStructMemberAccessor {
        public:
            AncestorStructMemberAccessor(std::unique_ptr<AbstractStructMemberAccessor> ancestor_accessor)
                : ancestor_accessor_{std::move(ancestor_accessor)} {}
            AncestorStructMemberAccessor(AncestorStructMemberAccessor const & x) = default;
            virtual ~AncestorStructMemberAccessor() = default;

            static std::unique_ptr<AncestorStructMemberAccessor>
            adopt(std::unique_ptr<AbstractStructMemberAccessor> ancestor_accessor) {
                return std::unique_ptr<AncestorStructMemberAccessor>
                    (new AncestorStructMemberAccessor(std::move(ancestor_accessor)));
            } /*adopt*/

            void * address_impl(StructT * self_addr) const {
                /* to use access-via-ancestor,  need to convert to ancestor pointer */
                AncestorT * ancestor_addr = self_addr;

                return this->ancestor_accessor_->address(ancestor_addr);
            } /*address_impl*/

            // ----- inherited from AbstractStructMemberAccessor -----

#ifdef OBSOLETE
            virtual TaggedPtr member_tp(void * struct_addr) const override {
                AncestorT * ancestor_addr = reinterpret_cast<StructT *>(struct_addr);

                return this->ancestor_accessor_->member_tp(ancestor_addr);
            } /*member_tp*/
#endif

            virtual TypeDescr struct_td() const override { return EstablishTypeDescr::establish<StructT>(); }
            virtual TypeDescr member_td() const override { return this->ancestor_accessor_->member_td(); }

            virtual void * address(void * struct_addr) const override {
                return this->address_impl(reinterpret_cast<StructT *>(struct_addr));
            }

            virtual std::unique_ptr<AbstractStructMemberAccessor> clone() const override {
                return std::unique_ptr<AbstractStructMemberAccessor>
                    (new AncestorStructMemberAccessor(this->ancestor_accessor_->clone()));
            } /*clone*/

            virtual bool same_member(AbstractStructMemberAccessor const & x) const override {
                auto const * other = dynamic_cast<AncestorStructMemberAccessor const *>(&x);

                return other && this->ancestor_accessor_->same_member(*(other->ancestor_accessor_));
            } /*same_member*/

        private:
            /* .ancestor_accessor fetches some particular member of AncestorT */
            std::unique_ptr<AbstractStructMemberAccessor> ancestor_accessor_;
        }; /*AncestorStructMemberAccessor*/

        template <typename StructT> class StructMemberDecl;
        template <typename StructT> class StructReflector;

        /* describes a member of a struct/class
         * see [reflect/StructReflector.hpp]
         */
        class StructMember {
        public:
            StructMember() = default;
            /* ownership: the member type's default (child_edge_ownership()), so
             * that type must be described already -- Reflect::require'd, not
             * just established
             */
            StructMember(std::string const & name,
                         std::unique_ptr<AbstractStructMemberAccessor> accessor)
                : member_name_{name},
                  accessor_{std::move(accessor)},
                  ownership_{default_ownership(accessor_.get())} {}
            StructMember(StructMember && x)
                : member_name_{std::move(x.member_name_)},
                  accessor_{std::move(x.accessor_)},
                  ownership_{x.ownership_},
                  guard_ix_{x.guard_ix_} {}

            static StructMember null();

            std::string const & member_name() const { return member_name_; }
            /** how the struct relates to what this member leads to: the
             *  member itself if held by value (always owning), its pointee
             *  if a pointer.  The member type's default unless the struct's
             *  reflection overrode it (StructMemberDecl).  See Ownership.hpp
             **/
            Ownership ownership() const { return ownership_; }
            /** index of the lockable guarding this member, in its struct's
             *  guard table (TypeDescr::guard_tp()); none if unguarded.  See
             *  StructMemberDecl::guarded_by
             **/
            std::optional<uint32_t> guard_ix() const { return guard_ix_; }

            TaggedPtr get_member_tp(void * struct_addr) const { return this->accessor_->member_tp(struct_addr); }
            TypeDescr get_struct_td() const { return this->accessor_->struct_td(); }
            TypeDescr get_member_td() const { return this->accessor_->member_td(); }
            //void * get_member_addr(void * struct_addr) const { return this->accessor_->address(struct_addr); }

            /* make copy that accesses this member,  but starting
             * from pointer to some derived class DescendantT,
             * instead of from container type StructT known to (but not exposed by) *this
             */
            template <typename DescendantT, typename StructT>
            StructMember for_descendant() const {
                assert(EstablishTypeDescr::establish<StructT>() == this->get_struct_td());

                StructMember retval(this->member_name(),
                                    std::move(AncestorStructMemberAccessor<DescendantT, StructT>::adopt
                                              (this->accessor_->clone())));
                /* keep an override from the ancestor's reflection */
                retval.ownership_ = this->ownership_;
                /* an index into the ancestor's guard table: the adopter
                 * (StructReflector::adopt_parent) remaps it into its own
                 */
                retval.guard_ix_ = this->guard_ix_;

                return retval;
            } /*for_descendant*/

            StructMember & operator=(StructMember && x) {
                this->member_name_ = std::move(x.member_name_);
                this->accessor_ = std::move(x.accessor_);
                this->ownership_ = x.ownership_;
                this->guard_ix_ = x.guard_ix_;
                return *this;
            }

        private:
            template <typename StructT> friend class StructMemberDecl;
            template <typename StructT> friend class StructReflector;

            static Ownership default_ownership(AbstractStructMemberAccessor const * accessor) {
                if (!accessor)
                    return Ownership::owning;

                TypeDescr td = accessor->member_td();

                assert(td->tdextra() && "StructMember: member type must be described (Reflect::require)");

                return td->tdextra() ? td->child_edge_ownership() : Ownership::owning;
            }

        private:
            /* member name, e.g. foo if
             *   struct StructT { MemberT foo; }
             */
            std::string member_name_;
            /* T recd;
             * this->accessor_->address_impl(&recd) ==> &(recd.member)
             */
            std::unique_ptr<AbstractStructMemberAccessor> accessor_;
            /* see .ownership() */
            Ownership ownership_ = Ownership::owning;
            /* see .guard_ix() */
            std::optional<uint32_t> guard_ix_;
        }; /*StructMember*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end StructMember.hpp */
