/* @file String.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#include "xo/alloc/Object.hpp"
#include "ObjectConversion.hpp"
#include "xo/allocutil/IAlloc.hpp"
#include "xo/indentlog/print/tag.hpp"

namespace xo {
    namespace obj {
        /** unicode terminology (via https://utf8everywhere.org)
         *  1. code unit:
         *     bit combination that represents a unit of encoded text.
         *     8-bits for utf-8 i.e. code-unit = char
         *  2. code point:
         *     a numerical value in the unicode namespace, e.g. U+3243F
         *     one or more code units encode a code point.
         *     utf-8 uses 1-4 code units to encode each code point.
         *  3. abstract character:
         *     inherently open, because includes characters that are not
         *     (yet) representable in unicode.
         *  4. (en)coded character:
         *     mapping between code points and abstract character.
         *     for example U+1F428 is coded character for emoji named 'KOALA'
         *     caveats:
         *     - some code points do not have abstract characters assigned.
         *     - some code points are reserved for non-characters
         *       (e.g. null, newline ..)
         *     - some abstract characters require multiple code points:
         *       for example requiring a composition sequence
         *     - some abstract characters have multiple encodings
         *  5. user-perceived character. whatever you think that means.
         *     May be language-dependent.
         *  6. grapheme cluster. a sequence of coded characters that
         *     "belong together". for example backspace would erase
         *     a grapheme cluster atomically.
         *  7. a shape within a font.  A sequence of code points maps to
         *     a sequence of glyphs.
         **/
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

            /** Approximate number of columns (if using a fixed-width font) occupied
             *  by this string. Obtained by counting bytes up to null terminator,
             *  omitting utf-8 continuation bytes, i.e. bytes with high bit set
             *  and 2nd-highest bit clear.
             *
             *  @text
             *         1 byte
             *        <------>
             *  bits: 76543210
             *        10______
             *  @endtext
             **/
            std::size_t columns() const;

            std::strong_ordering operator<=>(const String & other) const {
                size_t len1 = std::strlen(chars_);
                size_t len2 = std::strlen(other.chars_);

                return std::lexicographical_compare_three_way(chars_, chars_ + len1,
                                                              other.chars_, other.chars_ + len2);
            }

            bool operator==(const String & other) const {
                size_t len1 = std::strlen(chars_);
                size_t len2 = std::strlen(other.chars_);

                if (len1 != len2)
                    return false;

                return std::memcmp(chars_, other.chars_, len1) == 0;
            }

            // inherited from Object..
            virtual TaggedPtr self_tp() const final override;
            virtual void display(std::ostream & os) const final override;
            virtual std::size_t _shallow_size() const final override;
            virtual Object * _shallow_copy(gc::IAlloc * gc) const final override;
            virtual std::size_t _forward_children(gc::IAlloc * gc) final override;

        private:
            String(owner owner, std::size_t z, char * s);
            /** create instance, copying string contents (when @p copy_flag is true) using allocator @p mm **/
            String(gc::IAlloc * mm, owner owner, std::size_t z, char * s);

        private:
            /** true iff storage in @ref chars_ is owned by this String.
             **/
            owner owner_ = owner::shared;
            /** length of @ref chars_ in bytes (storage allocated, not necessarily string length).
             *  Includes null terminator
             **/
            std::size_t z_chars_ = 0;
            /** utf-8 string contents.  always null-terminated.
             *  Note that this is #of bytes
             **/
            char * chars_ = nullptr;
        };

        struct ObjectConversion_String {
            static gp<Object> to_object(gc::IAlloc * mm, std::string x) {
                return String::copy(mm, x.c_str());
            }
            static std::string from_object(gc::IAlloc *, gp<Object> x) {
                gp<String> x_str = String::from(x);
                if (x_str.get()) {
                    /* note: ignores allocator, always uses heap.
                     *       This will affect operation of primitives (if any) that
                     *       expect std::string. Alternative would be use IAlloc*,
                     *       with Blob wrapper (or without if/when need for iterable
                     *       memory is dropped).
                     */
                    return std::string(x_str->c_str());
                } else {
                    throw std::runtime_error
                        (tostr("ObjectConversion_String"
                               ": x found where string expected", xtag("x", x)));
                }

            }
        };

        template <>
        struct ObjectConversion<std::string> : public ObjectConversion_String {};

    } /*namespace obj*/
} /*namespace xo*/

/* end String.hpp */
