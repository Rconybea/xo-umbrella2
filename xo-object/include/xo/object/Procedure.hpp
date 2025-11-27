/** @file Procedure.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "Object.hpp"

namespace xo {
    namespace obj {
        /** @class ProcedureInterface
         *  @brief Interface to a dynamically-typed procedure
         **/
        class Procedure : public Object {
        public:
            virtual std::size_t n_args() const = 0;
            virtual gp<Object> apply_nocheck(const CVector<gp<Object>> & args) = 0;

            // inherited from Object..

            // virtual TaggedPtr self_tp() const override;
            // virtual void display(std::ostream &) const override;
            // virtual size_t _shallow_size() const override;
            // virtual Object * _shallow_copy(gc::IAlloc *) const override;
            // virtual size_t _forward_children(gc::IAlloc *) override;
        };
    }
}

/* end Procedure.hpp */
