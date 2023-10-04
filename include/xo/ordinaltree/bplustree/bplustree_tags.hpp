/* @file bplustree_tags.hpp */

#pragma once

namespace xo {
    namespace tree {
        namespace tags {
            /* ordinal_enabled: compute ordinal statistics;
             *                  in particular maintain per-node subtree size
             */
            enum ordinal_tag { ordinal_enabled, ordinal_disabled };
        } /*tags*/
    } /*namespace tree*/
} /*namespace xo*/

/* end bplustree_tags.hpp */
