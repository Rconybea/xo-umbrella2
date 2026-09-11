/** @file PrettySinkFactory.hpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#pragma once

#include <xo/ppsink/PpSinkFactory.hpp>
#include "xo/indentlog2/print/PpConfig.hpp"

namespace xo::pp {
    /** @brief factory that creates pretty-printing sinks for logging.
     *
     *  Sinks created by this factory forward to clog's streambuf
     **/
    class PrettySinkFactory : public PpSinkFactory {
    public:
        explicit PrettySinkFactory(const PpConfig & x) : pp_config_{x} {}

        const PpConfig & pp_config() const { return pp_config_; }

        virtual bool is_flat() const override { return false; }
        /** create pretty-printing instance **/
        virtual std::unique_ptr<PpSink> create() override;

    private:
        /** pretty-printing configuration **/
        PpConfig pp_config_;
    };
} /*namespace xo::pp*/

/* end PrettySinkFactory.hpp */
