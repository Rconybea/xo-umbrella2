/** @file VisitReason.hpp
 *
 *  @author Roland Conybeare, Apr 2026
 **/

#pragma once

namespace xo {
    namespace mm {

        /** @brief tag when navigating object graph
         *
         *  Used with
         *    @ref DX1Collector::visit_child
         *    @ref GCObject::visit_gco_children
         **/
        class VisitReason {
        public:
            enum class code {
                invalid = -1,

                /** color not needed **/
                unspecified,

                /** Forward child pointers inplace for GC.
                 *  See @ref GCObjectStore::forward_inplace_aux
                 **/
                forward,
                /** verify GC store consistency
                 *  See @ref DX1Collector::_verify_aux
                 **/
                verify,

                N,
            };

            explicit VisitReason(code x) : code_{x} {}

            static VisitReason unspecified() { return VisitReason(code::unspecified); }
            static VisitReason forward() { return VisitReason(code::forward); }
            static VisitReason verify() { return VisitReason(code::verify); }

            code code() const noexcept { return code_; }

            enum code code_;
        };

        inline bool operator==(VisitReason x, VisitReason y) { return x.code() == y.code(); }
        inline bool operator!=(VisitReason x, VisitReason y) { return x.code() != y.code(); }

    } /*namespace mm*/
} /*namespace xo*/

/* end VisitReason.hpp */
