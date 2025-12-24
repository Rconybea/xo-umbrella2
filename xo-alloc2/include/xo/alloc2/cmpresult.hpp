/** @file cmpresult.hpp
*
 *  @author Roland Conybeare, Dec 2025
 **/

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
    namespace mm {
        enum class comparison : int32_t {
            invalid      = -1,
            comparable   =  0,
            incomparable = +1,
        };

        extern const char * comparison2str(comparison x);

        inline std::ostream &
        operator<<(std::ostream & os, comparison x) {
            os << comparison2str(x);
            return os;
        }

        /** Result of a generic comparison operation
         **/
        struct cmpresult {
            /** @defgroup mm-cmpresult-ctors cmpresult ctors **/
            ///@{
            cmpresult() : err_{comparison::invalid}, cmp_{0} {}
            cmpresult(comparison err, std::int16_t cmp) : err_{err}, cmp_{cmp} {}

            static cmpresult incomparable() { return cmpresult(comparison::incomparable,  0); }
            static cmpresult       lesser() { return cmpresult(comparison::comparable,   -1); }
            static cmpresult        equal() { return cmpresult(comparison::comparable,    0); }
            static cmpresult      greater() { return cmpresult(comparison::comparable,   +1); }
            template<typename T>
            static cmpresult from_cmp(T && x, T && y) {
                if (x < y)
                    return cmpresult::lesser();
                else if (x == y)
                    return cmpresult::equal();
                else
                    return cmpresult::greater();
            }

            ///@}

            /** @defgroup mm-cmpresult-methods cmpresult methods **/
            ///@{

            /** print to stream **/
            void display(std::ostream & os) const;

            bool is_lesser() const {
                return (err_ == comparison::comparable) && (cmp_ < 0);
            }
            bool is_equal() const {
                return (err_ == comparison::comparable) && (cmp_ == 0);
            }
            ///@}

            /** @defgroup mm-cmpresult-instance-vars cmpresult instance vars **/
            ///@{
            /** -1 -> invalid (sentinel)
             *   0 -> comparable
             *  +1 -> incomparable (e.g. iterators from different arenas)
             **/
            comparison   err_ = comparison::invalid;
            /** <0 -> lesser; 0 -> equal, >0 -> greater **/
            std::int16_t cmp_ = 0;
            ///@}
        };

        inline std::ostream & operator<<(std::ostream & os,
                                         const cmpresult & x)
        {
            x.display(os);
            return os;
        }

    } /*namespace mm*/
} /*namespace xo*/

/* end cmpresult.hpp */
