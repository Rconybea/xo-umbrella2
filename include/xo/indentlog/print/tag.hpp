/* @file tag.hpp */

#pragma once

#include "tag_config.hpp"
#include "concat.hpp"
#include "quoted.hpp"
#include "color.hpp"
#include <iostream>

// STRINGIFY(xyz) -> "xyz"
#ifndef STRINGIFY
# define STRINGIFY(x) #x
#endif

// TAG(xyz) -> tag("xyz", xyz)
#define TAG(x) xo::make_tag(STRINGIFY(x), x)
#define TAG2(x, y) xo::make_tag(x, y)

#define XTAG(x) xo::xtag(STRINGIFY(x), x)
//#define XTAG2(x, y) xo::xtag(x, y)

namespace xo {
    // associate a name with a value
    //
    // will print like
    //   :name value
    //
    // NOTE: will search for operator<< overloads in the logutil
    //       namespace
    //*/
    template <bool PrefixSpace, typename Name, typename Value>
    struct tag_impl {
        tag_impl(Name const & n, Value const & v)
            : name_{n}, value_{v} {}
        tag_impl(Name && n, Value && v)
            : name_{std::forward<Name>(n)}, value_{std::forward<Value>(v)} {}

        Name const & name() const { return name_; }
        Value const & value() const { return value_; }

    private:
        Name name_;
        Value value_;
    }; /*tag_impl*/

    /* deduce tag template-type from arguments */
    template<typename Name, typename Value>
    tag_impl<false, Name, Value>
    make_tag(Name && n, Value && v)
    {
        return tag_impl<false, Name, Value>(n, v);
    } /*make_tag*/

    template<typename Value>
    tag_impl<false, char const *, Value>
    make_tag(char const * n, Value && v) {
        return tag_impl<false, char const *, Value>(n, v);
    } /*make_tag*/

    template<typename Name, typename Value>
    tag_impl<true, Name, Value>
    xtag(Name && n, Value && v)
    {
        return tag_impl<true, Name, Value>(n, v);
    } /*xtag*/

    template<typename Value>
    tag_impl<true, char const *, Value>
    xtag(char const * n, Value && v) {
        return tag_impl<true, char const *, Value>(n, v);
    } /*xtag*/

    inline
    tag_impl<true, char const *, char const *>
    xtag_pre(char const * n) {
        return tag_impl<true, char const *, char const *>(n, "");
    } /*xtag_pre*/

    // ----- tag -----

    template<typename Name, typename Value>
    tag_impl<false, Name, Value>
    tag(Name && n, Value && v)
    {
        return tag_impl<false, Name, Value>(n, v);
    } /*tag*/

    template<typename Value>
    tag_impl<false, char const *, Value>
    tag(char const * n, Value && v)
    {
        return tag_impl<false, char const *, Value>(n, v);
    } /*tag*/

    // ----- operator<< on tag_impl -----

    template <bool PrefixSpace, typename Name, typename Value>
    inline std::ostream &
    operator<<(std::ostream &s,
               tag_impl<PrefixSpace, Name, Value> const & tag)
    {
        using xo::print::unq;

        if(PrefixSpace)
            s << " ";

        s << with_color(tag_config::tag_color, concat((char const *)":", tag.name()))
          << " " << unq(tag.value());

        return s;
    } /*operator<<*/
} /*namespace xo*/

/* end tag.hpp */
