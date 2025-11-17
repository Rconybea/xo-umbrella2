/** @file StackFrame.hpp **/

#include "xo/alloc/IAlloc.hpp"
#include "xo/alloc/Object.hpp"
#include <cstdint>

namespace xo {
    namespace scm {
        /** @class StackFrame
         *  @brief Represent a single runtime stack frame for a Schematika function
         *
         *  StackFrame intended to be used for interpreted functions.
         *  Compiled functions will still likely have stack frames, but need not use the
         *  @ref StackFrame class
         *
         *  memory layout:
         *
         *     +------------+
         *     | vtable     |
         *     +------------+
         *     | .n_        |
         *     +------------+
         *     | .v_      +------\
         *     +------------+ <--/
         *     | .v_[0]     |
         *     +------------+
         *     .     ..     .
         *     +------------+
         *     | .v_[.n_-1] |
         *     +------------+
         **/
        class StackFrame : public Object {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            StackFrame(gc::IAlloc * mm, std::size_t n_slot);

            /** create frame using allocator @p mm,
             *  with exactly @p n_slot object pointers
             **/
            static gp<StackFrame> make(gc::IAlloc * mm, std::size_t n_slot);

            /** reflect StackFrame object representation **/
            static void reflect_self();

            std::size_t n_slot() const { return n_slot_; }
            gp<Object> lookup(std::size_t i) const { return v_[i]; }
            gp<Object> & lookup(std::size_t i) { return v_[i]; }

            gp<Object> operator[](std::size_t i) const { return lookup(i); }
            gp<Object> & operator[](std::size_t i) { return lookup(i); }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy() const final override;
            virtual std::size_t _forward_children() final override;

        private:
            /** number of elements in frame **/
            std::size_t n_slot_ = 0;
            /** contiguous array of object pointers: v[0] .. v[n-1] **/
            gp<Object> * v_ = nullptr;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end StackFrame.hpp */
