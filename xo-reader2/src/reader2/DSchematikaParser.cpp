/** @file DSchematikaParser.cpp
 *
 *  @author Roland Conybeare, Jan 2026
 **/

#include "DToplevelSeqSsm.hpp"
#include "ParserStack.hpp"
#include "ParserStateMachine.hpp"
#include "SchematikaParser.hpp"
#include <xo/indentlog2/print/tostr.hpp>
#include <xo/ppsink/scope.hpp>
#include <xo/ppsink/scope_macros.hpp>
#include <xo/ppsink/pretty_struct.hpp>
/* os << xtag(..); see the ostream-containment milestone */
#include <xo/ppsink/tag_ostream.hpp>
#include <cstddef>
#include <stdexcept>

namespace xo {
    using xo::pp::scope;
    using xo::mm::ACollector;
    using xo::mm::AAllocator;
    using xo::mm::AGCObject;
    using xo::mm::MemorySizeInfo;
    using xo::pp::tostr;
    using xo::pp::xtag;

    namespace scm {
        // ----- SchematikaParser -----

        DSchematikaParser::DSchematikaParser(const ParserConfig & cfg,
                                             obj<AAllocator> expr_alloc,
                                             obj<AAllocator> aux_alloc)
        : psm_{
                cfg.parser_arena_config_,
                cfg.symtab_var_config_,
                cfg.symtab_types_config_,
                cfg.max_stringtable_capacity_,
                cfg.pm_install_flags_,
                expr_alloc,
                aux_alloc
            },
          debug_flag_{cfg.debug_flag_}
        {
        }

        DSchematikaParser *
        DSchematikaParser::_make(obj<AAllocator> mm,
                                 const ParserConfig & cfg,
                                 obj<AAllocator> expr_alloc,
                                 obj<AAllocator> aux_alloc)
        {
            void * mem = mm.alloc_for<DSchematikaParser>();

            return new (mem) DSchematikaParser(cfg, expr_alloc, aux_alloc);
        }

        obj<AGCObject,DSchematikaParser>
        DSchematikaParser::make(obj<AAllocator> mm,
                                const ParserConfig & cfg,
                                obj<AAllocator> expr_alloc,
                                obj<AAllocator> aux_alloc)
        {
            return obj<AGCObject,DSchematikaParser>(_make(mm, cfg, expr_alloc, aux_alloc));
        }

        DGlobalSymtab *
        DSchematikaParser::global_symtab() const noexcept
        {
            return psm_.global_symtab();
        }

        DGlobalEnv *
        DSchematikaParser::global_env() const noexcept
        {
            return psm_.global_env();
        }

        bool
        DSchematikaParser::is_at_toplevel() const
        {
            return psm_.is_at_toplevel();
        }

        bool
        DSchematikaParser::has_incomplete_expr() const
        {
            return psm_.has_incomplete_expr();
        }

        obj<ASyntaxStateMachine>
        DSchematikaParser::top_ssm() const
        {
            return psm_.top_ssm();
        }

        const ParserResult &
        DSchematikaParser::result() const
        {
            return psm_.result();
        }

        void
        DSchematikaParser::visit_pools(const MemorySizeVisitor & visitor) const
        {
            return psm_.visit_pools(visitor);
        }

        void
        DSchematikaParser::begin_interactive_session()
        {
            DToplevelSeqSsm::establish_interactive(psm_.parser_alloc(), &psm_);

        }

        void
        DSchematikaParser::begin_batch_session()
        {
            DToplevelSeqSsm::establish_batch(psm_.parser_alloc(), &psm_);
        }

        const DUniqueString *
        DSchematikaParser::intern_string(std::string_view str)
        {
            return psm_.intern_string(str);
        }

        const ParserResult &
        DSchematikaParser::on_token(const token_type & tk)
        {
            scope log(XO_DEBUG_(debug_flag_), xtag("tk", tk));

            if (psm_.stack() == nullptr) {
                throw std::runtime_error(tostr("DSchematikaParser::include_token",
                                               ": parser not expecting input"
                                               "(call parser.begin_translation_unit()..?)",
                                               xtag("token", tk)));
            }

            /* stack is non-empty */

            psm_.on_token(tk);

            log && log(xtag("parser", this));
            log && log(xtag("result", psm_.result()));

            return psm_.result();
        } /*include_token*/

        void
        DSchematikaParser::reset_result()
        {
            psm_.reset_result();
        }

        void
        DSchematikaParser::reset_to_idle_toplevel()
        {
            psm_.clear_error_reset();
        } /*reset_to_idle_toplevel*/

        void
        DSchematikaParser::pretty(xo::pp::PpSink & sink) const
        {
            /* force-break because SchematikaParser is big */
            auto st = sink.struct_open("SchematikaParser", true /*force_break*/);

            st.field("stack", psm_.stack());
        }

        DSchematikaParser *
        DSchematikaParser::gco_shallow_move(obj<AGCObjectVisitor> gc) noexcept
        {
            (void)gc;

            /** TODO: may be feasible to use gc.std_move_for(this)
             *  if/when DSchematikaParser is moveable
             **/
            assert(false);
            return nullptr;
        }

        void
        DSchematikaParser::visit_gco_children(VisitReason reason,
                                              obj<AGCObjectVisitor> gc) noexcept
        {
            psm_.visit_gco_children(reason, gc);
        }

    } /*namespace scm*/
} /*namespace xo*/

/* end SchematikaParser.cpp */
