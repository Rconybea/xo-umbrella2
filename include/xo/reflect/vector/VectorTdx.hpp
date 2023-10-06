/* file VectorTdx.hpp
 *
 * author: Roland Conybeare, Aug 2022
 */

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
#include "xo/reflect/EstablishTypeDescr.hpp"

namespace xo {
  namespace reflect {
    /* Extra type-associated information for a vector/array.
     */
    class VectorTdx : public TypeDescrExtra {
    public:
      /* named ctor idiom.  create new instance for a vector type */
      //static std::unique_ptr<VectorTdx> make();

      // ----- Inherited from TypeDescrExtra -----

      virtual Metatype metatype() const override { return Metatype::mt_vector; }
      virtual uint32_t n_child(void * object) const override = 0;
      virtual TaggedPtr child_tp(uint32_t i, void * object) const override = 0;
      /* (forbidden) */
      virtual std::string const & struct_member_name(uint32_t i) const override;
    }; /*VectorTdx*/

    // ----- StlVectorTdx -----

    /* require:
     * - VectorT.size()
     * - VectorT[int] :: lvalue
     */
    template<typename VectorT>
    class StlVectorTdx : public VectorTdx {
    public:
      using target_t = VectorT;

      static std::unique_ptr<StlVectorTdx> make() {
    return std::unique_ptr<StlVectorTdx>(new StlVectorTdx());
      } /*make*/

      virtual uint32_t n_child(void * object) const override {
    target_t * vec = reinterpret_cast<target_t *>(object);

    return vec->size();
      } /*n_child*/

      virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
    target_t * vec = reinterpret_cast<target_t *>(object);

    return establish_most_derived_tp(&((*vec)[i]));
      } /*child_tp*/
    }; /*StlVectorTdx*/

    // ----- std::array<Element, N> -----

    /* coordinates with EstablishTdx<std::array<Element, N>>::make(),
     * see [reflect/Reflect.hpp]
     */

    template<typename Element, std::size_t N>
    using StdArrayTdx = StlVectorTdx<std::array<Element, N>>;

    // ----- std::vector<Element> -----

    /* coordinates with EstablishTdx<std::vector<Element>>::make()
     * see [reflect/Reflect.hpp]
     */
    template<typename Element>
    class StdVectorTdx : public VectorTdx {
    public:
      using target_t = std::vector<Element>;

      static std::unique_ptr<StdVectorTdx> make() {
    return std::unique_ptr<StdVectorTdx>(new StdVectorTdx());
      } /*make*/

      virtual uint32_t n_child(void * object) const override {
    target_t * vec = reinterpret_cast<target_t *>(object);

    return vec->size();
      } /*n_child*/

      virtual TaggedPtr child_tp(uint32_t i, void * object) const override {
    target_t * vec = reinterpret_cast<target_t *>(object);

    return establish_most_derived_tp(&((*vec)[i]));
      }
    }; /*StdVectorTdx*/
  } /*namespace reflect*/

} /*namespace xo*/

/* end VectorTdx.hpp */
