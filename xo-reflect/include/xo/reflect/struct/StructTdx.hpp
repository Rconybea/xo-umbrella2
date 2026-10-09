/* @file StructTdx.hpp */

#pragma once

#include "StructMember.hpp"
#include "xo/reflect/TaggedPtr.hpp"
#include "xo/reflect/TypeDescrExtra.hpp"
// #include "xo/reflect/struct/StructMember.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace xo {
    namespace reflect {
        /* Extra type-associated information for a struct/class.
         * We use this to preserve information about memory layout
         * at runtime
         */
        class StructTdx : public TypeDescrExtra {
        public:
            /** accessors to the lockables guarding a struct's members **/
            using GuardVector = std::vector<std::unique_ptr<AbstractStructMemberAccessor>>;

            /* named ctor idiom.  create new instance for struct with given member list
             *
             * guard_v.  lockables guarding members: StructMember::guard_ix()
             *           indexes it
             * parent_v. parent types (StructReflector::adopt_parent)
             * to_self_tp.  use this function to support .most_derived_self_tp()
             */
            static std::unique_ptr<StructTdx> make(std::vector<StructMember> member_v,
                                                   GuardVector guard_v,
                                                   std::vector<TypeDescr> parent_v,
                                                   bool have_to_self_tp,
                                                   std::function<TaggedPtr (void *)> to_self_tp);

            /* specialization for std::pair<Lhs, Rhs>
             * coordinates with [reflect/Reflect.hpp]
             */
            template<typename Lhs, typename Rhs>
            static std::unique_ptr<StructTdx> pair() {
                using struct_t = std::pair<Lhs, Rhs>;

                std::vector<StructMember> mv;
                {
                    auto lhs_access
                        (GeneralStructMemberAccessor<struct_t, struct_t, Lhs>::make
                         (&struct_t::first));

                    mv.push_back(StructMember("first", std::move(lhs_access)));
                }
                {
                    auto rhs_access
                        (GeneralStructMemberAccessor<struct_t, struct_t, Rhs>::make
                         (&struct_t::second));

                    mv.push_back(StructMember("second", std::move(rhs_access)));
                }

                std::function<TaggedPtr (void *)> null_to_self_tp;

                return make(std::move(mv),
                            GuardVector(),
                            std::vector<TypeDescr>(),
                            false /*!have_to_self_tp*/,
                            null_to_self_tp);
            } /*pair*/

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_struct; }
            virtual TaggedPtr most_derived_self_tp(TypeDescrBase const * object_td,
                                                   void * object) const override {
                if (this->have_to_self_tp_) {
                    return this->to_self_tp_(object);
                } else {
                    return TypeDescrExtra::most_derived_self_tp(object_td, object);
                }
            }
            /* object argument ignored for structs,  since size is fixed */
            virtual uint32_t n_child(void * /*object*/) const override { return this->member_v_.size(); }
            virtual uint32_t n_child_fixed() const override { return this->member_v_.size(); }
            virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
            virtual TypeDescr fixed_child_td(uint32_t i) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            virtual StructMember const * struct_member(uint32_t i) const override;
            virtual uint32_t n_parent() const override { return this->parent_v_.size(); }
            virtual TypeDescr parent_td(uint32_t i) const override { return this->parent_v_.at(i); }
            virtual uint32_t n_guard() const override { return this->guard_v_.size(); }
            virtual TaggedPtr guard_tp(uint32_t g, void * object) const override;

            // ----- guards -----

            /** accessor to lockable @p g.  require: g < n_guard() **/
            AbstractStructMemberAccessor const & guard(uint32_t g) const { return *(this->guard_v_.at(g)); }
            /** members with no guard, in declaration order **/
            std::vector<uint32_t> const & unguarded_members() const { return this->unguarded_v_; }
            /** members guarded by lockable @p g, in declaration order **/
            std::vector<uint32_t> const & guard_members(uint32_t g) const { return this->guard_members_v_.at(g); }

        private:
            StructTdx(std::vector<StructMember> member_v,
                      GuardVector guard_v,
                      std::vector<TypeDescr> parent_v,
                      bool have_to_self_tp,
                      std::function<TaggedPtr (void*)> to_self_tp);

        private:
            /* per-instance-variable reflection details */
            std::vector<StructMember> member_v_;
            /* lockables guarding members; StructMember::guard_ix() indexes it */
            GuardVector guard_v_;
            /* parent types, as declared by StructReflector::adopt_parent */
            std::vector<TypeDescr> parent_v_;
            /* indices (into .member_v) of members with no guard */
            std::vector<uint32_t> unguarded_v_;
            /* .guard_members_v[g]: indices (into .member_v) of members guarded by .guard_v[g] */
            std::vector<std::vector<uint32_t>> guard_members_v_;
            /* true if .to_self_tp() is defined */
            bool have_to_self_tp_ = false;
            /* get TaggedPtr for most-derived subtype of supplied T-instance */
            std::function<TaggedPtr (void *)> to_self_tp_;
        }; /*StructTdx*/

    } /*namespace reflect*/
} /*namespace xo*/

/* end StructTdx.hpp */
