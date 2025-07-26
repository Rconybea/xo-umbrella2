/** @file type_unifier.cpp
 *
 *  author: Roland Conybeare, Jul 2025
 **/

#include "typeinf/type_unifier.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
    namespace scm {
        std::ostream &
        operator<< (std::ostream & os,
                    const unify_result & x)
        {
            os << "<unify_result"
               << xtag("success", x.success_)
               << xtag("unified", x.unified_);
            if (x.error_src_function_)
                os << xtag("error_src_function", x.error_src_function_);
            if (!x.error_description_.empty())
                os << xtag("error_description", x.error_description_);
            os << ">";
            return os;
        }

        unify_result
        type_unifier::occurs_error(const char * src_function,
                                   bp<TypeBlueprint> t1,
                                   bp<TypeBlueprint> t2,
                                   bp<TypeBlueprint> s1,
                                   bp<TypeBlueprint> s2)
        {
            // unification implies some infinite type,
            // e.g. unify a' with (i64 -> 'a)
            // would imply type (i64 -> i64 -> i64 -> ...)
            return {
                .success_ = false,
                .unified_ = nullptr,
                .error_src_function_ = src_function,
                .error_description_ = tostr("attempting unify(T1,T2) with T1 -> S1, T2 -> S2",
                                            ": occurs check failed with S1 occuring in S2",
                                            xrefrtag("T1", t1),
                                            xrefrtag("T2", t2),
                                            xrefrtag("S1", s1),
                                            xrefrtag("S2", s2))
            };
        };

        unify_result
        type_unifier::unify(bp<TypeBlueprint> lhs, bp<TypeBlueprint> rhs)
        {
            /** if we already have substitutions for either of {lhs, rhs}, use them **/

            auto lhs1 = lhs->substitute(constraint_map_);
            auto rhs1 = rhs->substitute(constraint_map_);

            /** reminder:
             *  1. lhs1, rhs1 need not be in constraint_map,
             *  2. lhs1, rhs1 need not be distinct from lhs, rhs respectively
             **/

            if (TypeBlueprint::equals(lhs1, rhs1)) {
                // blueprints are already equivalent on their face.
                // this recognizes matching concrete types.
                //
                // return the lexicographically earlier id as canonical representative
                bp<TypeBlueprint> canonical = (lhs1->id() < rhs1->id()) ? lhs1 : rhs1;

                return {
                    .success_ = true,
                    .unified_ = canonical.promote(),
                    .error_src_function_ = nullptr,
                    .error_description_ = ""
                };
            }

            assert(lhs1->id() != rhs1->id());

            constexpr const char * c_self_name = "type_unifier::unify";

            bp<TypeBlueprint> canonical;

            /** if both lhs1 and rhs1 are type variables,
             *  pick the lexicographically earlier one as canonical name.
             *  (already know they're distinct because did not satisfy equality test above)
             *
             *  prefer the canonical name as rhs target of all substitutions
             *  from known-to-be-equivalent typevars.
             **/
            if (lhs1->is_variable())
            {
                if (rhs1->is_variable())
                {
                    // haven't resolved anything yet, but we do know
                    // that type variables lhs,rhs,lhs1,rhs1 must refer to the same type

                    if (lhs1->ref().id() < rhs1->ref().id()) {
                        canonical = lhs1;
                        constraint_map_[rhs1->id()] = lhs1.promote();
                    } else {
                        canonical = rhs1;
                        constraint_map_[lhs1->id()] = rhs1.promote();
                    }
                } else if (rhs1->is_concrete()) {
                    canonical = (lhs1->id() < rhs1->id()) ? lhs1 : rhs1;

                    // update lhs, lhs1 to refer to resolved rhs1.
                    // rhs would already have been resolved
                    assert(rhs->td() == rhs1->td());

                    lhs1->resolve_to(rhs1->td());
                    if (lhs->id() != lhs1->id())
                        lhs->resolve_to(rhs1->td());
                } else {
                    // 1. lhs1->is_variable()
                    // 2. !rhs1->is_variable() && !rhs1->is_concrete()
                    //
                    // therefore need occurs check for lhs1 appearing in rhs1

                    std::set<type_var> rhs1_typevar_set;
                    rhs1->upsert_typevars(&rhs1_typevar_set);

                    if (rhs1_typevar_set.contains(lhs1->id())) {
                        return type_unifier::occurs_error(c_self_name,
                                                          lhs, rhs, lhs1, rhs1);
                    }

                    // TODO: some sort of recursive unification here
                    assert(false);
                }
            } else if (rhs1->is_variable())
            {
                assert(!rhs1->is_concrete());

                if (lhs1->is_concrete())
                {
                    canonical = (lhs1->id() < rhs1->id()) ? lhs1 : rhs1;

                    // update rhs, rhs1 to refer to resolved lhs1.
                    // lhs would already have been resolved
                    assert(lhs->td() == lhs1->td());

                    rhs1->resolve_to(lhs1->td());
                    if (rhs->td() != rhs1->td())
                        rhs->resolve_to(lhs1->td());
                } else
                {
                    // 1. !lhs1->is_variable() && !lhs1->is_concrete()
                    // 2. rhs1->is_variable()
                    //
                    // Need occurs check for rhs1 appearing in lhs1

                    std::set<type_var> lhs1_typevar_set;
                    lhs1->upsert_typevars(&lhs1_typevar_set);

                    if (lhs1_typevar_set.contains(rhs1->id())) {
                        return type_unifier::occurs_error(c_self_name,
                                                          rhs, lhs, rhs1, lhs1);
                    }

                    // TODO: some sort of recursive unification here
                    assert(false);
                }
            } else if (lhs1->is_concrete() && rhs1->is_concrete())
            {
                /* we already know lhs1 != rhs1 -> unification failure */
                return {
                    .success_ = false,
                    .unified_ = nullptr,
                    .error_src_function_ = c_self_name,
                    .error_description_ = tostr("attempting unify(T1,T2) with T1 -> S1, T2 -> S2",
                                                ": incompatible concrete types S1,S2",
                                                xrefrtag("T1", lhs),
                                                xrefrtag("T2", rhs),
                                                xrefrtag("S1", lhs1),
                                                xrefrtag("S2", rhs1))
                };
            }

            // TODO: recursive unification for structural types, function types etc.

            if (canonical)
            {
                constraint_map_[lhs1->id()] = canonical.promote();
                constraint_map_[rhs1->id()] = canonical.promote();

                if (!constraint_map_.contains(lhs1->id()))
                    constraint_map_[lhs1->id()] = canonical.promote();
                if (!constraint_map_.contains(rhs1->id()))
                    constraint_map_[rhs1->id()] = canonical.promote();

                return {
                    .success_ = true,
                    .unified_ = canonical.promote(),
                    .error_src_function_ = nullptr,
                    .error_description_ = ""
                };
            }

            assert(false);

            return {
                .success_ = false,
                .unified_ = nullptr,
                .error_src_function_ = c_self_name,
                .error_description_ = tostr("attempting unify(T1,T2) with T1 -> S1, T2 -> S2",
                                            "supposedly-unreachable case for S1,S2",
                                            xrefrtag("T1", lhs),
                                            xrefrtag("T2", rhs),
                                            xrefrtag("S1", lhs1),
                                            xrefrtag("S2", rhs1))
            };
        }

    } /*namespace scm*/
} /*namespace xo*/

/** end type_unifier.cpp **/
