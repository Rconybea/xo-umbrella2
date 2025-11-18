/** @file StackFrame.hpp **/

#include "xo/alloc/IAlloc.hpp"
#include "Env.hpp"
#include <cstddef>
#include <cstdint>

namespace xo {
    namespace scm {
        /** gc-only vector
         **/
        template <typename ElementType>
        class CVector {
        public:
            using value_type = ElementType;

        public:
            CVector(gc::IAlloc * mm, std::size_t n)
                : n_{n}, v_{nullptr}
                {
                    if (n_ > 0) {
                        std::byte * mem = mm->alloc(n_ * sizeof(ElementType));
                        this->v_ = new (mem) ElementType[n];
                    }
                }

            std::size_t size() const { return n_; }

            ElementType operator[](std::size_t i) const { return v_[i]; }
            ElementType & operator[](std::size_t i) { return v_[i]; }

            friend class StackFrame;
        private:
            /** number of elements in @ref v_ **/
            std::size_t n_ = 0;
            /** contiguous array of pointers **/
            ElementType * v_ = nullptr;
        };

        /** @class StackFrame
         *  @brief Represent a single runtime stack frame for a Schematika function
         *
         *  StackFrame intended to be used for interpreted functions.
         *  Compiled functions will still likely have stack frames, but need not use the
         *  @ref StackFrame class
         *
         *  memory layout:
         *                                  ^
         *     +-----------------------+    |
         *     | vtable                |    |
         *     +-----------------------+    |
         *     | .parent             +------/
         *     +------------+----------+
         *     | .slot_v_   | .n_      |
         *     |            +----------+
         *     |            | .v_    +------\
         *     +------------+----------+ <--/
         *     | .v_[0]              +---------> Object(1)
         *     +-----------------------+
         *     .            ..         .
         *     +-----------------------+
         *     | .v_[.n_-1]          +---------> Object(n)
         *     +-----------------------+
         **/
        class StackFrame : public Object {
        public:
            using TaggedPtr = xo::reflect::TaggedPtr;

        public:
            StackFrame(gc::IAlloc * mm, gp<StackFrame> p, std::size_t n) : parent_{p}, slot_v_{mm, n} {}

            /** create frame using allocator @p mm,
             *  with parent @p p and exactly @p n_slot object pointers
             **/
            static gp<StackFrame> make(gc::IAlloc * mm, gp<StackFrame> p, std::size_t n_slot);

            /** reflect StackFrame object representation **/
            static void reflect_self();

            gp<StackFrame> parent() const { return parent_; }
            std::size_t size() const { return slot_v_.size(); }

            gp<Object> operator[](std::size_t i) const { return slot_v_[i]; }
            gp<Object> & operator[](std::size_t i) { return slot_v_[i]; }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy() const final override;
            virtual std::size_t _forward_children() final override;

        private:
            /** parent stack frame **/
            gp<StackFrame> parent_;
            /** stack frame contents **/
            CVector<gp<Object>> slot_v_;
        };
    } /*namespace scm*/
} /*namespace xo*/

/* end StackFrame.hpp */
