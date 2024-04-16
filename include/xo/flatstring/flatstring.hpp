/** @file flatstring.hpp
 *
 *  Author: Roland Conybeare
 **/

#pragma once

#include <string_view>
#include <sstream>
#include <algorithm>
#include <memory>

namespace xo {
    /** @class flatstring
     *  @brief class to represent a string with a fixed amount of storage space.
     *
     *  - Flatstring memory layout is a fixed-size, null-terminated char array.
     *  - With a few exceptions, flatstring methods are noexcept.
     *    @c flatstring<N>::at() may throw,  for consistency with @c std::string::at() behavior
     *  - Construction and concatenation of flatstrings are constexpr,
     *    and can be done at compile time.
     *    We rely on this in related projects (e.g. https://github.com:rconybea/xo-unit)
     *  - Preserves as much of the c++23 @c std::string api as practicable
     *
     *  @c N includes mandatory null terminator,  so we require @c N > 0.
     *
     *  @invariant all flatstring instances are null-terminated.
     *  @invariant sizeof(flatstring<N>) == N
     **/
    template <std::size_t N>
    struct flatstring {
        /** @defgroup flatstring-types template types
         *  @brief Template types exposed by @c flatstring<N>
         **/
        ///@{
        /** @brief character traits for this flatstring **/
        using traits_type            = std::char_traits<char>;
        /** @brief type of each character in this flatstring **/
        using value_type             = char;
        using allocator_type         = std::allocator<char>;
        using size_type              = std::allocator_traits<allocator_type>::size_type;
        using difference_type        = std::allocator_traits<allocator_type>::difference_type;
        /** @brief type of a character reference **/
        using reference              = value_type &;
        /** @brief type of a readonly character reference **/
        using const_reference        = const value_type &;
        using pointer                = std::allocator_traits<allocator_type>::pointer;
        using const_pointer          = std::allocator_traits<allocator_type>::const_pointer;
        /** @brief representation for a read/write iterator **/
        using iterator               = char *;
        /** @brief representation for a readonly iterator **/
        using const_iterator         = const char *;
        /** @brief representation for a read/write reverse iterator **/
        using reverse_iterator       = char *;
        /** @brief representation for a readonly reverse iterator **/
        using const_reverse_iterator = const char *;
        ///@}

        /** @defgroup flatstring-constants constants **/
        ///@{
        static constexpr const size_type npos = size_type(-1);

        /** @brief capacity of this flatstring,  including final null terminator.
         *
         *  @note not present in @c std::string api
         **/
        static constexpr const std::size_t fixed_capacity = N;
        ///@}

    public:
        /** @defgroup flatstring-ctor constructors **/
        ///@{
        /** @brief create empty string literal.  Will contain N null characters
         *
         *  Example
         *  @code
         *    constexpr flatstring<5> s1;
         *    static_assert(s1.empty());
         *  @endcode
         **/
        constexpr flatstring() noexcept {
            /* note: clang verifies that we fully initialize memory;  otherwise will not recognize
             *       instance as constexpr
             */
            std::fill_n(value_, N, '\0');
        }

        /** @brief create string literal from a correctly-sized char array
         *
         *  Example
         *  @code
         *    constexpr flatstring s1("hello");
         *    static_assert(s1.size() > 0);
         *  @endcode
         **/
        constexpr flatstring(const char (&str)[N]) noexcept {
            std::copy_n(str, N, value_);
        }
        ///@}

        /** @defgroup flatstring-properties property-methods **/
        ///@{
        /** @brief true if (and only if) string is empty **/
        constexpr bool empty() const noexcept { return value_[0] == '\0'; }
        /** @brief returns current size of this string **/
        constexpr size_type size() const noexcept {
            return this->cend() - this->cbegin();
        }
        /** @brief synonym for @c size() **/
        constexpr size_type length() const noexcept { return size(); }

        constexpr size_type capacity() const noexcept { return fixed_capacity - 1; }
        constexpr size_type max_size() const noexcept { return fixed_capacity - 1; }

        /** @brief contents as plain old C-style string. **/
        constexpr const char * c_str() const noexcept { return value_; }
        ///@}

        /** @defgroup flatstring-access access methods **/
        ///@{
        /** @brief return char at position @p pos in this string (counting from zero).
         *
         *  Throws @c std::out_of_range exception if @p pos >= @c N
         **/
        constexpr value_type & at(size_type pos) throw() { return this->at_aux(pos); }
        constexpr const value_type & at(size_type pos) const throw() { return const_cast<flatstring *>(this)->at_aux(pos); }

        /** @brief return char at position @p pos in this string (counting from zero).
         *
         *  Does not check bounds: undefined behavior if @p pos >= @c N
         *
         *  @pre 0<=pos<=N-1
         **/
        constexpr value_type & operator[](size_type pos) { return value_[pos]; }
        constexpr const value_type & operator[](size_type pos) const { return value_[pos]; }
        ///@}

        /** @defgroup flatstring-iterators iterators **/
        ///@{
        constexpr iterator begin() { return &value_[0]; }
        constexpr iterator end() { return this->last(); }

        constexpr const_iterator cbegin() const { return &value_[0]; }
        constexpr const_iterator cend() const { return const_cast<flatstring*>(this)->last<iterator>(); }
        constexpr const_iterator begin() const { return cbegin(); }
        constexpr const_iterator end() const { return cend(); }

        constexpr reverse_iterator rbegin() { return this->last(); }
        constexpr reverse_iterator rend() { return &value_[0]; }
        constexpr const_reverse_iterator crbegin() const { return const_cast<flatstring*>(this)->last<iterator>(); }
        constexpr const_reverse_iterator crend() const { return &value_[0]; }
        constexpr const_reverse_iterator rbegin() const { return crbegin(); }
        constexpr const_reverse_iterator rend() const { return crend(); }
        ///@}

        /** @defgroup flatstring-assign assignment **/
        ///@{
        /** @brief put string into empty state.  fills entire char array with nulls **/
        void clear() { std::fill_n(value_, N, '\0'); }

        /** @brief replace contents with min(count,N-1) copies of character ch **/
        constexpr flatstring & assign(size_type count, value_type ch) {
            std::size_t pos = 0;
            for (; pos < std::min(count, N-1); ++pos)
                value_[pos] = ch;
            for (; pos < N; ++pos)
                value_[pos] = '\0';

            return *this;
        }
        /** @brief replace contents with first N-1 characters of str **/
        constexpr flatstring & assign(const flatstring & x) {
            for (std::size_t pos = 0; pos < N-1; ++pos)
                value_[pos] = x.value_[pos];
            value_[N-1] = '\0';
            return *this;
        }
        /** @brief replace contents with substring [pos,pos+count] of str **/
        constexpr flatstring & assign(const flatstring & x,
                                      size_type pos, size_type count = npos) {
            std::size_t i = 0;
            for (;
                 i < std::min(std::min(count,
                                       std::max(x.capacity-1 - pos,
                                                0)),
                              N-1);
                 ++i)
                value_[i] = x.value_[pos+i];
            for (; i < N; ++i)
                value_[i] = '\0';

            return *this;
        }
        /** @brief replace contents with range [cstr, cstr + count) **/
        constexpr flatstring & assign(const value_type * cstr, size_type count) {
            std::size_t i = 0;
            for (; i < std::min(N-1, count); ++i)
                value_[i] = cstr[i];
            for (; i < N; ++i)
                value_[i] = '\0';

            return *this;
        }
        /** @brief replace contents with C-style string cstr **/
        constexpr flatstring & assign(const value_type * cstr) {
            std::size_t i = 0;
            const value_type * p = cstr;
            while ((i < N-1) && (*p != '\0')) {
                value_[i] = *p;
                ++i;
                ++p;
            }
            for (; i < N; ++i)
                value_[i] = '\0';

            return *this;
        }
        /** @brief replace contents with iterator range [first, last) **/
        template <typename InputIter>
        constexpr flatstring & assign(InputIter first, InputIter last) {
            InputIter ix = first;
            std::size_t i = 0;
            for (; (i < N-1) && (ix != last); ++i) {
                value_[i] = *ix;
            }
            for (; i < N; ++i)
                value_[i] = '\0';
            return *this;
        }
        ///@}

        // insert
        // insert_range
        // erase
        // push_back
        // append
        // append_range
        // operator+=
        // replace
        // replace_with_range
        // copy
        // find
        // rfind
        // find_first_of
        // find_first_not_of
        // find_last_of
        // find_last_not_of
        // compare
        // starts_with
        // end_with
        // contains
        // substr

        /** @defgroup flatstring-conversion-operators conversion operators **/
        ///@{
        /** @brief conversion to @c std::string
         *
         *  Example
         *  @code
         *    constexpr flatstring s("bazinga!");
         *    std::string s_str{s.str()};
         *  @endcode
         **/
        std::string str() const { return std::string(value_); }

        /** @brief conversion operator to string_view **/
        constexpr operator std::string_view() const noexcept { return std::string_view(value_); }

        /** @brief conversion operator to C-style string.
         *
         *  Example
         *  @code
         *    constexpr flatstring s("obey gravity..");
         *    strcmp(s, "obey...");
         *  @endcode
         **/
        constexpr operator const char * () const { return value_; }
        ///@}

    private:
        constexpr value_type & at_aux(size_type pos) {
            if (pos >= N) {
#ifdef NOT_USING
                /* note: can't build stringstream at compile time */
                std::stringstream ss;
                ss << "flatstring<" << N << ">::at: expected pos=[" << pos << "] in interval [0," << N << ")" << std::endl;
#endif

                throw std::out_of_range("at_aux: range error");
            }

            return (*this)[pos];
        }

        template <typename Iterator>
        constexpr Iterator last() {
            Iterator p = &value_[N-1];

            /* search backward for first padding '\0' */
            while ((p > &value_[0]) && (*(p-1) == '\0'))
                --p;

            return p;
        }

    public:
        /** @defgroup flatstring-instance-variables instance variables **/
        ///@{

        /** @brief characters comprising this literal string **/
        char value_[N];

        ///@}
    };

    /** @brief sentinel type,  for forbidden stringliteral with no space for a null terminator **/
    template <>
    struct flatstring<0> { flatstring() = delete; };

    // non-member functions
    // erase
    // erase_if
    // operator<<
    // operator>>
    // getline
    // stoi
    // stol
    // stoll
    // stoul
    // stoull
    // stof
    // stod
    // stold

#ifdef NOT_USING
    /** @brief all_same_v<T1, .., Tn> is true iff types T1 = .. = Tn
     **/
    template < typename First, typename... Rest >
    constexpr auto
    all_same_v = std::conjunction_v< std::is_same<First, Rest>... >;
#endif

    /** @brief Concatenate flatstrings, possibly mixed with C-style char arrays
     *
     *  Example:
     *  @code
     *  constexpr auto s = flatstring_concat(flatstring("hello"),
     *                                       ", ",
     *                                       flatstring("world"));
     *  static_assert(s.capacity == 13);
     *  @endcode
     *
     **/
    template < typename... Ts>
    constexpr auto
    flatstring_concat(Ts && ... args) noexcept
    {
#ifdef NOT_USING
        static_assert(all_same_v<std::decay_t<Ts>...>,
                      "string must share the same char type");

        using char_type = std::remove_const_t< std::remove_pointer_t < std::common_type_t < Ts... > > >;
#endif
        using value_type = char;

        /** n1: total number of bytes used by arguments **/
        constexpr std::size_t n1 = (sizeof(Ts) + ...);
        /** z1: each string arg has a null terminator included in its size,
         *      z1 is the number of arguments in parameter pack Ts,
         *      which equals the number of null terminators used
         **/
        constexpr std::size_t z1 = sizeof...(Ts);

        /** n:  number of chars in concatenated string.  +1 for final null **/
        constexpr std::size_t n
            = (n1 / sizeof(value_type)) - z1 + 1;

        flatstring<n> result;

        std::size_t pos = 0;

        auto detail_concat = [ &pos, &result ](auto && arg) {
            constexpr auto count = (sizeof(arg) - sizeof(value_type)) / sizeof(value_type);

            std::copy_n(/*arg.c_str()*/ static_cast<const char *>(arg), count, result.value_ + pos);
            pos += count;
        };

        (detail_concat(args), ...);

        return result;
    }

    /** @brief compare two flatstrings lexicographically.
     *
     *  Example:
     *  @code
     *  constexpr auto cmp = flatstring_compare(stringliteral("foo"), stringliteral("bar"));
     *  static_assert(cmp > 0);
     *  @endcode
     **/
    template <std::size_t N1,
              std::size_t N2>
    constexpr auto
    flatstring_compare(const flatstring<N1> & s1,
                       const flatstring<N2> & s2) noexcept
    {
        return (std::string_view(s1.value_) <=> std::string_view(s2.value_));
    }

    /** @defgroup flatstring-3way-compare 3way-compare **/
    ///@{
    /** @brief 3-way compare for two flatstrings
     *
     *  Example
     *  @code
     *  constexpr auto cmp = (flatstring("foo") <=> flatstring("bar"));
     *  static_assert(cmp != 0);
     *  @endcode
     **/
    template <std::size_t N1,
              std::size_t N2>
    constexpr auto
    operator<=>(const flatstring<N1> & s1,
                const flatstring<N2> & s2) noexcept
    {
        return (std::string_view(s1) <=> std::string_view(s2));
    }
    ///@}
} /*namespace xo*/

/** end stringliteral.hpp **/
