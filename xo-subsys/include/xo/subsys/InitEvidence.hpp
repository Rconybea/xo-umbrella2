/** @file InitEvidence.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    template <typename Tag>
    class SubsystemImpl;

    /* evidence that one or more subsystems have been initialized.
     * Used to prevent static linker stripping must-run initialization code
     */
    class InitEvidence {
    public:
        using uint64_t = std::uint64_t;

    public:
        InitEvidence() = default;

        uint64_t evidence() const { return evidence_; }

        InitEvidence operator^=(InitEvidence x) {
            this->evidence_ ^= x.evidence_;

            return *this;
        } /*operator^=*/

        InitEvidence operator^(InitEvidence x) {
            return InitEvidence(this->evidence_ ^ x.evidence_);
        }

    private:
        /** only SubsystemImpl<Tag> should produce InitEvidence **/
        InitEvidence(uint64_t x) : evidence_{x} {}

        template <typename Tag>
        friend class SubsystemImpl;

    private:
        /* we don't care about the specific value computed here,
         * purpose is to be sufficiently impenentrable to compiler such
         * that static linker can't optimize it away
         */
        uint64_t evidence_ = 0;
    }; /*InitEvidence*/

} /*namespace xo*/

/* end InitEvidence.hpp */
