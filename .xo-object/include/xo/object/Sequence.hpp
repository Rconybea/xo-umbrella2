/* @file Sequence.hpp
 *
 * author: Roland Conybeare, Aug 2025
 */

#pragma once

#include "Collection.hpp"

namespace xo {
    namespace obj {
        class Sequence : public Collection {
            // inherited from Object..

            //virtual std::size_t _shallow_size() const override;
            //virtual Object * _shallow_copy() const override;
            //virtual std::size_t _fixup_forwarded_children() override;
        };
    } /*namespace obj*/
} /*namespace xo*/

/* end Sequence.hpp */
