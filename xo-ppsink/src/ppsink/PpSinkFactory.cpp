/** @file PpSinkFactory.cpp
 *
 *  @author Roland Conybeare, Sep 2026
 **/

#include "PpSinkFactory.hpp"
#include "FlatSink.hpp"
#include <iostream>

namespace xo::pp {

    PpSinkFactory &
    PpSinkFactory::set_instance(PpSinkFactory * x)
    {
        s_instance = x;
        return *s_instance;
    }

    /** process-wide default sink: flat output to std::clog.
     *  (POC: whole-program FlatSink; per-thread interleaving not yet addressed)
     **/
    class FlatSinkFactory : public PpSinkFactory {
    public:
        /** fallback factory: creates flat sinks **/
        virtual bool is_flat() const override { return true; }
        /** create FlatSink instance **/
        virtual std::unique_ptr<PpSink> create() override {
            /** low-dependency fallback. No pretty-printing **/
            return std::make_unique<FlatSink>(std::clog.rdbuf());
        }
    };

    FlatSinkFactory s_flatsink_factory;

    PpSinkFactory *
    PpSinkFactory::s_instance = &s_flatsink_factory;

} /*namespace xo::pp*/

/* end PpSinkFactory.cpp */
