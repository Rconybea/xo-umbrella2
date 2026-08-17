/** @file Refcounted_ostream.hpp
 **/

#pragma once

#include <xo/refcnt/Refcounted.hpp>

namespace xo::pp {

    template<typename T>
    inline std::ostream &
        operator<<(std::ostream & os, intrusive_ptr<T> const & x) {
        if (x.get()) {
            os << *(x.get());
        } else {
            os << "<nullptr " << reflect::type_name<T>() << ">";
        }
        return os;
    } /*operator<<*/

    template<typename T>
    inline std::ostream &
        operator<<(std::ostream & os, Borrow<T> x) {
        if (x) {
            os << *x;
        } else {
            os << "<nullptr "  << reflect::type_name<T>() << ">";
        }
        return os;
    } /*operator<<*/

} /*namespace xo::pp*/

/* end Refcounted_ostream.hpp */
