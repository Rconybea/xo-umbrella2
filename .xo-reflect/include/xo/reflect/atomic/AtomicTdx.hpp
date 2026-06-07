/* @file AtomicTdx.hpp */

#pragma once

#include "xo/reflect/TypeDescrExtra.hpp"
//#include "reflect/TaggedPtr.hpp"
#include <memory>

namespace xo {
    namespace reflect {
        class TaggedPtr;

        /* Extra type-associated information for an atomic type.
         * We use this as degenerate catch-all case for types that aren't known
         * to have additional structure (std::vector, std::map, int*, etc.)
         */
        class AtomicTdx : public TypeDescrExtra {
        public:
            virtual ~AtomicTdx() = default;

            static std::unique_ptr<AtomicTdx> make();

            // ----- Inherited from TypeDescrExtra -----

            virtual Metatype metatype() const override { return Metatype::mt_atomic; }
            virtual uint32_t n_child(void * /*object*/) const override { return 0; }
            virtual uint32_t n_child_fixed() const override { return 0; }
            virtual TaggedPtr child_tp(uint32_t /*i*/, void * /*object*/) const override;
            virtual const TypeDescrBase * fixed_child_td(uint32_t /*i*/) const override;
            virtual std::string const & struct_member_name(uint32_t i) const override;
            //virtual StructMember const * struct_member(uint32_t /*i*/) const override { return nullptr; }

        private:
            AtomicTdx() = default;
        }; /*TypeDescrExtra*/
    } /*namespace reflect*/
} /*namespace xo*/

/* end AtomicTdx.hpp */
