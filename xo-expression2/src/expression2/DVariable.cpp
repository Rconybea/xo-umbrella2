/** @file DVariable.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DVariable.hpp"
#include "exprtype.hpp"
#include <xo/indentlog/print/quoted.hpp>
#include <xo/ppsink/pretty_struct.hpp>   /* sink.pretty_struct(..), field(..) */
#include <xo/ppsink/quoted.hpp>          /* xo::pp::quot */
#include <cstddef>
#include <string_view>

namespace xo {
    using xo::facet::typeseq;

    namespace scm {

        DVariable *
        DVariable::make(obj<AAllocator> mm,
                        const DUniqueString * name,
                        const TypeRef & typeref,
                        Binding path)
        {
            void * mem = mm.alloc(typeseq::id<DVariable>(),
                                  sizeof(DVariable));

            return new (mem) DVariable(name, typeref, path);
        }

        DVariable::DVariable(const DUniqueString * name,
                             const TypeRef & typeref,
                             Binding path)
          : name_{name}, typeref_{typeref}, path_{path}
        {}

        void
        DVariable::assign_valuetype(TypeDescr td) noexcept
        {
            typeref_.resolve(td);
        }

        DVariable *
        DVariable::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            return gc.std_move_for(this);
        }

        void
        DVariable::visit_gco_children(VisitReason reason, obj<AGCObjectVisitor> gc) noexcept
        {
            typeref_.visit_gco_children(reason, gc);
        }

        bool
        DVariable::pretty_deprecated(const ppindentinfo & ppii) const
        {
            using xo::print::quot;

            auto name = (name_
                         ? std::string_view(*name_)
                         : std::string_view(""));

            return ppii.pps()->pretty_struct
                       (ppii,
                        "DVariable"
                        , refrtag("name", quot(name))
                        , refrtag("typeref", typeref_)
                           );
        }

        void
        DVariable::pretty(xo::pp::PpSink & sink) const
        {
            /* named locals: field() captures BY REFERENCE (pretty_struct.hpp) */
            auto name = (name_
                         ? std::string_view(*name_)
                         : std::string_view(""));
            const auto qname = xo::pp::quot(name);

            sink.pretty_struct("DVariable",
                               xo::pp::field("name", qname),
                               xo::pp::field("typeref", typeref_));
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end DVariable.cpp */
