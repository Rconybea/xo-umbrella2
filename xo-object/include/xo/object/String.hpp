/* @file String.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/IAlloc.hpp"
#include "xo/alloc/Object.hpp"

namespace xo {
    namespace obj {
        class String : public Object {
        public:
            enum class owner { unique, shared };

            /** donwcast from @p x iff x is actually a String. Otherwise nullptr **/
            static gp<String> from(gp<Object> x);

            /** create shared string @p s, using allocator @ref Object::mm **/
            static gp<String> share(const char * s);
            /** create shared string @p s, using allocator @p mm **/
            static gp<String> share(gc::IAlloc * mm, const char * s);
            /** create copy of string @p s, using allocator @ref Object::mm **/
            static gp<String> copy(const char * s);
            /** create copy of string @p s, using allocator @p mm **/
            static gp<String> copy(gc::IAlloc * mm, const char * s);

            /** create empty string with @p z bytes of string space **/
            static gp<String> allocate(std::size_t z);
            /** create string containing contents of @p s1 follwed by contents of @p s2 **/
            static gp<String> append(gp<String> s1, gp<String> s2);

            const char * c_str() const { return chars_; }
            std::size_t length() const;

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * mm) const final override;
            virtual std::size_t _forward_children() final override;

        private:
            String(owner owner, std::size_t z, char * s);
            /** create instance, copying string contents (when @p copy_flag is true) using allocator @p mm **/
            String(gc::IAlloc * mm, owner owner, std::size_t z, char * s);

        private:
            /** true iff storage in @ref chars_ is owned by this String.
             **/
            owner owner_ = owner::shared;
            /** length of @ref chars_ in bytes (storage allocated, not necessarily string length) **/
            std::size_t z_chars_ = 0;
            /** string contents.  always null-terminated **/
            char * chars_ = nullptr;
        };
    } /*namespace obj*/
} /*namespace xo*/

/* end String.hpp */
