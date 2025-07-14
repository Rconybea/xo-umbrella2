/* @file Sequence.cpp */

#include "Sequence.hpp"
#include "pretty_expression.hpp"
#include <cstddef>

namespace xo {
    namespace ast {
        std::set<std::string>
        Sequence::get_free_variables() const {
            std::set<std::string> retval;

            for (const auto & x : expr_v_) {
                std::set<std::string> free_vars;
                free_vars = x->get_free_variables();

                for (const auto & y : free_vars)
                    retval.insert(y);
            }

            return retval;
        }

        std::size_t
        Sequence::visit_preorder(VisitFn visitor_fn) {
            std::size_t n = 1;

            visitor_fn(this);

            for (const auto & x : expr_v_)
                n += x->visit_preorder(visitor_fn);

            return n;
        }

        std::size_t
        Sequence::visit_layer(VisitFn visitor_fn) {
            std::size_t n = 1;

            visitor_fn(this);

            for (const auto & x : expr_v_)
                n += x->visit_layer(visitor_fn);

            return n;
        }

        rp<Expression>
        Sequence::xform_layer(TransformFn xform_fn) {
            for (std::size_t i = 0, n = expr_v_.size(); i < n; ++i) {
                expr_v_[i] = expr_v_[i]->xform_layer(xform_fn);
            }

            return xform_fn(this);
        }

        void
        Sequence::attach_envs(bp<Environment> p) {
            for (const auto & x : expr_v_)
                x->attach_envs(p);
        }

        void
        Sequence::display(std::ostream & os) const {
            os << "<Sequence";
            std::size_t i = 0;
            for (const auto & x : expr_v_) {
                std::string i_str = tostr("[", i, "]");

                os << xtag(i_str.c_str(), x);
            }

            os << ">";
        }

        std::uint32_t
        Sequence::pretty_print(ppstate * pps, bool upto) const
        {
            if (upto) {
                std::uint32_t saved = pps->pos();

                if (!pps->has_margin())
                    return false;

                if (!pps->print_upto("<Sequence"))
                    return false;

                std::size_t i = 0;
                for (const auto & expr_i : expr_v_) {
                    if (!pps->has_margin())
                        return false;

                    std::string i_str = tostr("[", i, "]");

                    if (!pps->print_upto(xtag(i_str.c_str(), expr_i)))
                        return false;
                    ++i;
                }

                if (!pps->has_margin())
                    return false;

                return pps->scan_no_newline(saved);
            } else {
                std::uint32_t ci0 = pps->lpos();
                std::uint32_t ci1 = ci0 + pps->indent_width();

                pps->write("<Sequence");

                std::size_t i = 0;
                for (const auto & expr_i : expr_v_) {
                    std::string i_str = tostr("[", i, "]");

                    pps->newline_indent(ci1);
                    pps->pretty(xtag(i_str.c_str(), expr_i));
                    ++i;
                }

                pps->write(">");
                return false;
            }
        }
    } /*namespace scm*/
} /*namespace xo*/


/* end Sequence.cpp */
