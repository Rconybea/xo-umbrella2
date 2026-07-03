/** @file UtestRehearser.cpp
 *
 *  @author Roland Conybeare, Jul 2026
 **/

#include "UtestRehearser.hpp"

namespace xo {

    auto
    UtestRehearser::iterator::operator++() -> iterator &
    {
        if (parent_)
            ++(parent_->attention_);

        if (parent_->ok_flag_ && (parent_->attention_ == 1)) {
            /* skip 2nd pass */
            ++(parent_->attention_);
        }

        if (parent_->attention_ == 2)
            parent_ = nullptr;

        return *this;
    }



}

/* end UtestRehearser.cpp */
