/** @file Symbol.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "xo/alloc/Object.hpp"
#include <cstdint>

namespace xo {
    /** @class Symbol
     *  @brief represent a schematika symbol.
     *
     *  Each uniquely-spelled variable associates to a distinct symbol.
     *
     **/
    namespace scm {
        class Symbol : public Object {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            /** create new Symbol instance with name @p n **/
            static gp<Symbol> make(gc::IAlloc * mm, const char * n);

            // ----- reflection helpers ------

            virtual TaggedPtr self_tp() const override final;
            virtual void display(std::ostream & os) const override final;

            // ----- GC helpers -----
            virtual std::size_t _shallow_size() const override final;
            virtual Object * _shallow_copy() const override final;
            virtual std::size_t _forward_children() override final;

        private:
            Symbol(gc::IAlloc * mm, const char * name);

        private:
            /** name of this symbol **/
            const char * name_;
            /** size in bytes (= ascii chars) for this symbol. includes null terminator **/
            std::uint32_t name_z_;
        };
    } /*namespace scm*/
} /*namesapce xo*/

/* end Symbol.hpp */
