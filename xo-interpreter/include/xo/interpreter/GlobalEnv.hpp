/** @file GlobalEnv.hpp **/

#pragma once

#include "Env.hpp"
#include "xo/alloc/IAlloc.hpp"
#include "xo/expression/GlobalSymtab.hpp"

namespace xo {
    namespace scm {
        /** @class GlobalEnv
         *  @brief Top-level global environment
         **/
        class GlobalEnv : public Env {
        public:
            /** Create top-level global environment, allocating via @p mm.
             *  Expect one of these per interpreter session.
             **/
            static gp<GlobalEnv> make_empty(gc::IAlloc * mm,
                                            const rp<GlobalSymtab> & symtab);

            gc::IAlloc * get_mm() const { return mm_; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy() const final override;
            virtual std::size_t _forward_children() final override;

        private:
            GlobalEnv(gc::IAlloc * mm, const rp<GlobalSymtab> & symtab);

        private:
            /** memory manager to use **/
            gc::IAlloc * mm_ = nullptr;

            /** global symbol table.
             *  variables known to @c symtab_ are represented by
             *  corresponding values in @p slot_map_
             **/
            rp<GlobalSymtab> symtab_;

            /** environment contents.
             *  expression @c symtab_->lookup_binding(vname)
             *  has associated value @c slot_map_.at(vname)
             *
             *  TODO: replace with something subject to GC ?
             *        every member of @ref slot_map_ will have to be a
             *        GC root
             *
             *  TODO: probably want to hash here instead.
             *        May also want lhs names to be separately hashed symbols
             **/
            std::map<std::string, gp<Object>> slot_map_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end GlobalEnv.hpp */
