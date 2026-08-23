/** @file DExpectQDictSsm.cpp
 *
 *  @author Roland Conybeare, Mar 2026
 **/

#include "ExpectQDictSsm.hpp"
#include "ExpectQLiteralSsm.hpp"
#include <xo/stringtable2/String.hpp>
#include <xo/alloc2/GCObject.hpp>
#include <xo/ppsink/pretty_struct.hpp>  /* sink.pretty_struct(..), field(..) */

namespace xo {
    using xo::pp::field;
    using xo::print::APrintable;

    namespace scm {

        const char *
        QDictXst::_descr(enum code x)
        {
            switch (x) {
            case code::invalid: break;
            case code::qdict_0: return "qdict_0";
            case code::qdict_1a: return "qdict_1a";
            case code::qdict_1b: return "qdict_1b";
            case code::qdict_1c: return "qdict_1c";
            case code::qdict_1d: return "qdict_1d";
            case code::qdict_2: return "qdict_2";
            case code::N: break;
            }

            return "?QDictXst";
        }

        DExpectQDictSsm::DExpectQDictSsm() : state_{QDictXst::code::qdict_0} {}

        obj<ASyntaxStateMachine,DExpectQDictSsm>
        DExpectQDictSsm::make(DArena & parser_mm)
        {
            return obj<ASyntaxStateMachine,DExpectQDictSsm>(_make(parser_mm));
        }

        DExpectQDictSsm *
        DExpectQDictSsm::_make(DArena & parser_mm)
        {
            void * mem = parser_mm.alloc_for<DExpectQDictSsm>();

            return new (mem) DExpectQDictSsm();
        }

        void
        DExpectQDictSsm::start(ParserStateMachine * p_psm)
        {
            DArena::Checkpoint ckp = p_psm->parser_alloc().checkpoint();

            p_psm->push_ssm(ckp, DExpectQDictSsm::make(p_psm->parser_alloc()));
        }

        syntaxstatetype
        DExpectQDictSsm::ssm_type() const noexcept {
            return syntaxstatetype::expect_qdict;
        }

        std::string_view
        DExpectQDictSsm::get_expect_str() const {
            switch (state_.code()) {
            case QDictXst::code::qdict_0:
                return "leftbrace";
            case QDictXst::code::qdict_1a:
                return "symbol|rightbrace";
            case QDictXst::code::qdict_1b:
                return "colon";
            case QDictXst::code::qdict_1c:
                return "literal";
            case QDictXst::code::qdict_1d:
                return "semicolon|rightbrace";
            case QDictXst::code::qdict_2:
                return "(done)";
            case QDictXst::code::invalid:
            case QDictXst::code::N:
                break;
            }

            return "?DExpectQDictSsm";
        }

        void
        DExpectQDictSsm::on_leftbrace_token(const Token & tk,
                                            ParserStateMachine * p_psm)
        {
            if (state_.code() == QDictXst::code::qdict_0) {
                constexpr DDictionary::size_type hint_cap = 8;

                this->state_ = QDictXst(QDictXst::code::qdict_1a);
                this->dict_ = DDictionary::empty(p_psm->expr_alloc(), hint_cap);

                return;
            }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_rightbrace_token(const Token & tk,
                                             ParserStateMachine * p_psm)
        {
            if ((state_.code() == QDictXst::code::qdict_1a)
                || (state_.code() == QDictXst::code::qdict_1d))
                {

                    this->state_ = QDictXst(QDictXst::code::qdict_2);

                    obj<AGCObject> lit = obj<AGCObject,DDictionary>(dict_);

                    p_psm->pop_ssm();
                    p_psm->on_quoted_literal(lit);
                    return;
                }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_symbol_token(const Token & tk,
                                         ParserStateMachine * p_psm)
        {
            if (state_.code() == QDictXst::code::qdict_1a) {
                this->state_ = QDictXst(QDictXst::code::qdict_1b);
                this->key_ = DString::from_view(p_psm->expr_alloc(), std::string_view(tk.text()));
                return;
            }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_colon_token(const Token & tk,
                                        ParserStateMachine * p_psm)
        {
            if (state_.code() == QDictXst::code::qdict_1b) {
                this->state_ = QDictXst(QDictXst::code::qdict_1c);

                DExpectQLiteralSsm::start(p_psm,
                                          false /*!cxl_on_rightparen*/);
                return;
            }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_semicolon_token(const Token & tk,
                                            ParserStateMachine * p_psm)
        {
            if (state_.code() == QDictXst::code::qdict_1d) {
                this->state_ = QDictXst(QDictXst::code::qdict_1a);
                return;
            }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_token(const Token & tk,
                                  ParserStateMachine * p_psm)
        {
            switch(tk.tk_type())
            {
            case tokentype::tk_leftbrace:
                this->on_leftbrace_token(tk, p_psm);
                return;

            case tokentype::tk_symbol:
                this->on_symbol_token(tk, p_psm);
                return;

            case tokentype::tk_rightbrace:
                this->on_rightbrace_token(tk, p_psm);
                return;

            case tokentype::tk_colon:
                this->on_colon_token(tk, p_psm);
                return;

            case tokentype::tk_semicolon:
                this->on_semicolon_token(tk, p_psm);
                return;

            case tokentype::tk_rightparen:
            case tokentype::tk_comma:
            case tokentype::tk_lambda:
            case tokentype::tk_def:
            case tokentype::tk_deftype:
            case tokentype::tk_if:
            case tokentype::tk_singleassign:
            case tokentype::tk_string:
            case tokentype::tk_f64:
            case tokentype::tk_i64:
            case tokentype::tk_bool:
            case tokentype::tk_invalid:
            case tokentype::tk_quote:
            case tokentype::tk_leftparen:
            case tokentype::tk_leftbracket:
            case tokentype::tk_rightbracket:
            case tokentype::tk_leftangle:
            case tokentype::tk_rightangle:
            case tokentype::tk_cmple:
            case tokentype::tk_cmpge:
            case tokentype::tk_dot:
            case tokentype::tk_doublecolon:
            case tokentype::tk_assign:
            case tokentype::tk_yields:
            case tokentype::tk_plus:
            case tokentype::tk_minus:
            case tokentype::tk_star:
            case tokentype::tk_slash:
            case tokentype::tk_cmpeq:
            case tokentype::tk_cmpne:
            case tokentype::tk_nil:
            case tokentype::tk_type:
            case tokentype::tk_then:
            case tokentype::tk_else:
            case tokentype::tk_let:
            case tokentype::tk_in:
            case tokentype::tk_end:
            case tokentype::N:
                break;
            }

            Super::illegal_token(tk, p_psm);
        }

        void
        DExpectQDictSsm::on_quoted_literal(obj<AGCObject> lit,
                                           ParserStateMachine * p_psm)
        {
            if (state_.code() == QDictXst::code::qdict_1c) {
                // adjoin (key,value) pair into dictionary

                this->state_ = QDictXst(QDictXst::code::qdict_1d);

                assert(dict_);

                bool ok = dict_->upsert(p_psm->expr_alloc(), DDictionary::pair_type(key_, lit));

                this->key_ = nullptr;

                if (!ok) assert(false);

                return;
            }

            Super::illegal_quoted_literal(lit, p_psm);
        }

        void
        DExpectQDictSsm::pretty(xo::pp::PpSink & sink) const
        {
            obj<APrintable,DDictionary> dict_pr(dict_);

            /* named local: get_expect_str() returns BY VALUE. */
            const auto expect = this->get_expect_str();

            /* :key is a PER-FIELD optional -- legacy's three-arg
             * refrtag("key", key_, key_).
             *
             * dict_pr is built by DIRECT construction rather than a registry
             * lookup, which is why a null dict_ ABORTS here (empty obj<>,
             * uninitialized vtable slot) where its DExpectQList/QArraySsm
             * siblings merely throw.  Same defect as
             * .xo-backlog/xo-expression2/issues/02-dtypename-null-type-aborts.md
             * Pre-existing on both protocols; reproduced, not fixed.
             *
             * Legacy's unused `obj<AGCObject,DDictionary> dict(dict_)` is
             * dropped -- nothing read it.
             */
            sink.pretty_struct("DExpectQDictSsm",
                               field("state", state_),
                               field("expect", expect),
                               field("key", key_, bool(key_)),
                               field("dict", dict_pr));
        }


        void
        DExpectQDictSsm::visit_gco_children(VisitReason reason,
                                            obj<AGCObjectVisitor> gc) noexcept
        {
            gc.visit_child(reason, &key_);
            gc.visit_child(reason, &dict_);
        }

    } /*namespace scm*/

    namespace pp {
        XO_PRETTIFIER_VIA_CONVERSION(xo::scm::QDictXst, xo::scm::QDictXst::_descr)
    }
} /*namespace xo*/

/* end DExpectQDictSsm.cpp */
