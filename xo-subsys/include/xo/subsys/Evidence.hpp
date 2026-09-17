/** @file Evidence.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <cstdint>

namespace xo {
    template <typename Evidence>
    class EvidenceProvider;

    /** Evidence for something identified by @tp Proposition **/
    template <typename Proposition>
    class Evidence {
    public:
        using uint64_t = std::uint64_t;

    public:
        Evidence(const Evidence &) = default;
        Evidence() = delete;

        uint64_t evidence() const { return evidence_; }

    private:
        explicit Evidence(uint64_t x) : evidence_{x} {}

        friend class EvidenceProvider<Evidence>;

    private:
        uint64_t evidence_ = 0;
    };

    /** Something that credible produces evidence.
     *
     *  Goal is to allowing cooperating code to recruit compiler
     *  to verify that some evidence provider anchors a chain
     *  of trust.
     **/
    template <typename Evidence>
    class EvidenceProvider : public Evidence {
    public:
        EvidenceProvider(const EvidenceProvider &) = delete;
        EvidenceProvider(uint64_t secret) : Evidence(secret ^ 0xa5a5a5a5a5a5a5a5), secret_{secret} {}

    private:
        uint64_t secret_ = 0;
    };

} /*namespace xo*/

/* end Evidence.hpp */
