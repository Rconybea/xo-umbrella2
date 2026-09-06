/** @file ITop_Any.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include "ATop.hpp"
#include "xo/facet/typeseq.hpp"
#include "xo/facet/obj.hpp"

namespace xo::facet {
    class ITop_Any;

    template <>
    struct FacetImplementation<ATop, DVariantPlaceholder> {
        using ImplType = ITop_Any;
    };

    class ITop_Any : public ATop {
    public:
        // from ATop

        typeseq _typeseq() const noexcept override { return s_typeseq; }

        // LCOV_EXCL_START
        void _drop(Opaque) const noexcept override { _fatal(); }

    private:
        [[noreturn]] static void _fatal();
        // LCOV_EXCL_STOP

    public:
        static typeseq s_typeseq;
        static bool _valid;

    };
} /*namespace xo::facet*/

/* end ITop_Any.hpp */
