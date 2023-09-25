/* @file StructTdx.hpp */

#pragma once

#include "reflect/TypeDescrExtra.hpp"
#include "reflect/TaggedPtr.hpp"
#include "reflect/struct/StructMember.hpp"
#include <vector>
#include <functional>
#include <memory>

namespace xo {
  namespace reflect {
    /* Extra type-associated information for a struct/class.
     * We use this to preserve information about memory layout
     * at runtime
     */
    class StructTdx : public TypeDescrExtra {
    public:
      /* named ctor idiom.  create new instance for struct with given member list
       *
       * to_self_tp.  use this function to support .most_derived_self_tp()
       */
      static std::unique_ptr<StructTdx> make(std::vector<StructMember> member_v,
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
      virtual uint32_t n_child(void * /*object*/) const override { return this->member_v_.size(); }
      virtual TaggedPtr child_tp(uint32_t i, void * object) const override;
      virtual std::string const & struct_member_name(uint32_t i) const override;
      virtual StructMember const * struct_member(uint32_t i) const override;

    private:
        StructTdx(std::vector<StructMember> member_v,
                  bool have_to_self_tp,
                  std::function<TaggedPtr (void*)> to_self_tp)
            : member_v_{std::move(member_v)},
              have_to_self_tp_{have_to_self_tp},
              to_self_tp_{std::move(to_self_tp)} {}

    private:
      /* per-instance-variable reflection details */
      std::vector<StructMember> member_v_;
      /* true if .to_self_tp() is defined */
      bool have_to_self_tp_ = false;
      /* get TaggedPtr for most-derived subtype of supplied T-instance */
      std::function<TaggedPtr (void *)> to_self_tp_;
    }; /*StructTdx*/

  } /*namespace reflect*/
} /*namespace xo*/

/* end StructTdx.hpp */
