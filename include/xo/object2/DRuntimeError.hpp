/** @file DRuntimeError.hpp
 *
 *  @author Roland Conybeare, Feb 2026
 **/

#pragma once

#include "String.hpp"
#include <xo/facet/obj.hpp>
#include <xo/alloc2/Allocator.hpp>

namespace xo {
    namespace scm {

        /** @brief representation for runtime errors
         **/
        class DRuntimeError {
        public:
            using AGCObject = xo::mm::AGCObject;
            //using ACollector = xo::mm::ACollector;
            using AGCObjectVisitor = xo::mm::AGCObjectVisitor;
            using VisitReason = xo::mm::VisitReason;
            using AAllocator = xo::mm::AAllocator;
            using ppindentinfo = xo::print::ppindentinfo;

        public:
            /** convenience shortcut.**/
            static obj<AGCObject,DRuntimeError> make(obj<AAllocator> mm,
                                                     const char * src_fn,
                                                     const char * error_descr);

            /** create instance using memory from allocator @p mm
             *  @p src_fn identifies the (c++) function/method in which
             *  error detercted.
             *  @p error_descr contains human-readable error message;
             *  will be copied by this function.
             **/
            static DRuntimeError * _make(obj<AAllocator> mm,
                                         DString * src_fn,
                                         DString * error_descr);

            DString * src_function() const noexcept { return src_function_; }
            DString * error_descr() const noexcept { return error_descr_; }

            /** @defgroup scm-runtimeerror-printable-facet printable facet **/
            ///@{

            bool pretty(const ppindentinfo & ppii) const;

            ///@}
            /** @defgroup scm-runtimeerror-gcobject-facet gcobject facet **/
            ///@{

            DRuntimeError * gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept;
            void visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept;

            ///@}

        private:
            DRuntimeError(DString * src_fn, DString * error_descr);

        private:
            /** source location at which error identified **/
            DString * src_function_ = nullptr;
            /** error description (allocated from ErrorArena) **/
            DString * error_descr_ = nullptr;
        };

    } /*namespace scm*/
} /*namespace xo*/

/* end DRuntimeError.hpp */
