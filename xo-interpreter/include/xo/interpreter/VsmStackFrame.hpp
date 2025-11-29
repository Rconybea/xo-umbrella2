/** @file VsmStackFrame.hpp
 *
 *  @author Roland Conybeare, Nov 2025
 **/

#pragma once

#include "VsmInstr.hpp"
#include "xo/object/CVector.hpp"
#include "xo/alloc/Object.hpp"

namespace xo {
    namespace scm {
        /** @class VsmStackFrame
         *  @brief Virtual Schematika Machine stack frame
         *
         *  Intending to use the "cheney on the MTA" strategy,
         *  i.e. allocate frames using GC's bump allocator.
         *
         *  Parallels LocalEnv, but VSM implementation isn't reflected
         **/
        class VsmStackFrame : public Object {
        public:
            VsmStackFrame(gc::IAlloc * mm, gp<VsmStackFrame> p, std::size_t n, const VsmInstr * cont);

            /** create frame using allocator @p mm,
             *  with parent @p p and exactly @p n_slot object pointers.
             **/
            static gp<VsmStackFrame> make(gc::IAlloc * mm,
                                          gp<VsmStackFrame> p,
                                          std::size_t n_slot,
                                          const VsmInstr * cont);

            /** create new stack frame using allocator @p mm,
             *  with parent frame @p p; new frame contains values @p s0
             **/
            static gp<VsmStackFrame> push1(gc::IAlloc * mm,
                                           gp<VsmStackFrame> p,
                                           gp<Object> s0,
                                           const VsmInstr * cont);

            /** create new stack frame using allocator @p mm,
             *  with parent frame @p p; new frame contains values @p s0, @p s1
             **/
            static gp<VsmStackFrame> push2(gc::IAlloc * mm,
                                           gp<VsmStackFrame> p,
                                           gp<Object> s0,
                                           gp<Object> s1,
                                           const VsmInstr * cont);


            /** reflect VsmStackFrame object representation **/
            static void reflect_self();

            gp<VsmStackFrame> parent() const { return parent_; }
            std::size_t size() const { return slot_v_.size(); }
            const obj::CVector<gp<Object>> & argv() const { return slot_v_; }
            const VsmInstr * continuation() const { return cont_; }

            gp<Object> operator[](std::size_t i) const { return slot_v_[i]; }
            gp<Object> & operator[](std::size_t i) { return slot_v_[i]; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc *) const final override;
            virtual std::size_t _forward_children(gc::IAlloc *) final override;

        private:
            /** parent stack frame **/
            gp<VsmStackFrame> parent_;

            /** stored state **/
            obj::CVector<gp<Object>> slot_v_;

            /** proceed to this continuation when popping this frame **/
            const VsmInstr * cont_ = nullptr;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end VsmStackFrame.hpp */
