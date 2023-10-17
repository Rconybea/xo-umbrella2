/* @file websock_utest_main.cpp */

#include "websock/Webserver.hpp"
#include "volfit/init_volfit.hpp"
#include "volfit/Volfit.hpp"
#include "volfit/VolfitInputCapture.hpp"
#include "filter/init_filter.hpp"
#include "filter/KalmanFilterSvc.hpp"
#include "option/StrikeSetOmd.hpp"
#include "option/StrikeSetMarketModel.hpp"
#include "option/UlMarketModel.hpp"
#include "option/PricingContext.hpp"
#include "option/OptionStrikeSet.hpp"
#include "process/init_process.hpp"
#include "process/RealizationSource.hpp"
#include "process/RealizationTracer.hpp"
#include "process/UpxEvent.hpp"
#include "process/ExpProcess.hpp"
#include "process/BrownianMotion.hpp"
#include "simulator/init_simulator.hpp"
#include "simulator/Simulator.hpp"
#include "reactor/EventStore.hpp"
#include "randomgen/random_seed.hpp"
#include "randomgen/xoshiro256.hpp"
#include "time/Time.hpp"
#include "printjson/PrintJson.hpp"
#include "indentlog/print/tag.hpp"
#include <signal.h>

/* webserver instance */
static xo::ref::rp<xo::web::Webserver> g_ws;

void sigint_handler(int /*sig*/) {
    std::cerr << "main thread interrupt_handler\n";

    if (g_ws)
        g_ws->interrupt_stop_webserver();
}

int
main(int argc, char **argv) {
    using xo::web::Webserver;
    using xo::web::WebserverConfig;
    using xo::json::PrintJsonSingleton;
    using xo::vf::Volfit;
    using xo::vf::VolfitInputCaptureSvc;
    using xo::kalman::KalmanFilterStateExt;
    using xo::kalman::KalmanFilterSvc;
    using xo::kalman::KalmanFilterSpec;
    using xo::option::FlatVolsfc;
    using xo::option::BboTick;
    using xo::option::StrikeSetOmd;
    using xo::option::StrikeSetMarketModel;
    using xo::option::UlMarketModel;
    using xo::option::PricingContext;
    using xo::option::OptionStrikeSet;
    using xo::option::Secid;
    using xo::option::Pxtick;
    using xo::process::UpxEvent;
    using xo::process::RealizationSource;
    using xo::process::RealizationTracer;
    using xo::process::StochasticProcess;
    using xo::process::ExpProcess;
    using xo::process::BrownianMotion;
    using xo::sim::Simulator;
    using xo::reactor::PtrEventStore;
    using xo::reactor::StructEventStore;
    using xo::reactor::ReactorSource;
    using xo::rng::Seed;
    using xo::rng::xoshiro256ss;
    using xo::ref::rp;
    using xo::time::utc_nanos;
    using xo::time::timeutil;
    using xo::json::PrintJsonSingleton;
    using xo::json::PrintJson;
    using xo::Subsystem;
    using xo::scope;
    using xo::xtag;

    try {
#ifdef NOT_USING
        InitSubsys<S_option_tag>::require();
        InitSubsys<S_process_tag>::require();
        InitSubsys<S_reactor_tag>::require();
#endif

        XO_SUBSYSTEM_REQUIRE(volfit);
        XO_SUBSYSTEM_REQUIRE(filter);
        XO_SUBSYSTEM_REQUIRE(simulator);
        XO_SUBSYSTEM_REQUIRE(process);

        Subsystem::initialize_all();

        signal(SIGINT, sigint_handler);

        scope log(XO_ENTER0(always));

        rp<PrintJson> pjson = PrintJsonSingleton::instance();

        /* RC Sep 2022 - adding c++ translation of kalman/src/pywebsock/ex_websock.py;
         *               intending to debug server segfault without complication of running
         *               from python interpreter
         */

        Secid secid0(0, 0);
        Secid ul0 = Secid::ul(0);

        utc_nanos t0 = timeutil::ymd_hms_usec(20220926 /*ymd*/,
                                              93000 /*hms*/,
                                              0 /*usec*/);
        utc_nanos t1 = t0 + std::chrono::hours(30 * 24);

        /* sim = pysimulator.Simulator.make() */
        rp<Simulator> sim = Simulator::make(t0);

        /* ss = pyoption.make_option_strike_set(3, secid0, 10, 1, t1, Pxtick.penny_nickel) */
        rp<OptionStrikeSet> ss
            = OptionStrikeSet::regular(3 /*n*/,
                                       secid0 /*start_id*/,
                                       10.0 /*lo_strike*/,
                                       1.0 /*d_strike*/,
                                       t1 /*expiry*/,
                                       Pxtick::penny_nickel);

        /* cx = pyoption.make_pricing_context(t0, 11.11, .5, .06) */
        rp<PricingContext> cx
            = PricingContext::make(t0, 11.11 /*ref_spot*/,
                                   FlatVolsfc::make(0.5) /*volatility*/,
                                   0.06 /*rate*/);

        /* TODO: replace with constant to get deterministic behavior */
        Seed<xoshiro256ss> seed;

        /* ebm = pyprocess.make_exponential_brownian_motion(t0, 11.0, 0.5) */
        rp<ExpProcess> ebm
            = ExpProcess::make(11.0,
                               BrownianMotion<xoshiro256ss>::make(t0,
                                                                  0.5 /*volatility*/,
                                                                  seed));

        /* src = pyprocess.make_realization_source(ebm, dt.timedelta(seconds=1)) */
        rp<ReactorSource> src
            = RealizationSource<UpxEvent, double>::make(RealizationTracer<double>::make(ebm),
                                                        std::chrono::seconds(1));
        src->set_name("src");

        /* (A)
         * ulm = pyoption.make_ul_market_model(ul0, cx)
         */
        rp<UlMarketModel> ulm = UlMarketModel::make(ul0, cx);
        ulm->set_name("ulm");

        /* (B)
         * ssm = pyoption.make_strikeset_market_model(ss, cx)
         */
        rp<StrikeSetMarketModel> ssm = StrikeSetMarketModel::make(ss, cx);
        ssm->set_name("ssm");

        /* (C)
         * ssmd = pyoption.make_strikeset_omd(ss)
         */
        rp<StrikeSetOmd> ssmd = StrikeSetOmd::make(ss);
        ssmd->set_name("ssmd");

        /* (D)
         * bbos = pyoption.BboTickStore.make()
         */
        using BboTickStore = StructEventStore<BboTick>;
        rp<BboTickStore> bbos = StructEventStore<BboTick>::make();
        bbos->set_name("bbos");

        /* (E)
         * vfin = pyvolfit.make_volfit_input_capture(cx)
         */
        rp<VolfitInputCaptureSvc> vfin = VolfitInputCaptureSvc::make(cx);
        vfin->set_name("vfin");

        /* (F)
         * kfspec = pyvolfit.make_kf_spec_m1(t0, s0=0.3, p0=1.0, q=5)
         */
        KalmanFilterSpec kfspec = Volfit::kf_spec_m1(t0, 0.3, 1.0, 5.0);
        /* kf = pyfilter.make_kalman_filter(spec=kfspec) */
        rp<KalmanFilterSvc> kf = KalmanFilterSvc::make(kfspec);
        kf->set_debug_sim_flag(true);
        kf->set_name("kf");

        using KalmanFilterStateEventStore
            = PtrEventStore<rp<KalmanFilterStateExt>>;

        /* (G)
         * kfs = pyfilter.KalmanFilterStateEventStore.make()
         */
        rp<KalmanFilterStateEventStore> kfs
            = KalmanFilterStateEventStore::make();

        /* sim.add_source(src) */
        sim->add_source(src);

        /* uls = pyprocess.UpxEventStore.make() */
        using UpxEventStore = StructEventStore<UpxEvent>;
        rp<UpxEventStore> uls = UpxEventStore::make();
        src->attach_sink(uls);

        /* (A) */
        sim->add_source(ulm);
        src->attach_sink(ulm);

        /* (B) */
        sim->add_source(ssm);
        src->attach_sink(ssm);

        /* (C) */
        sim->add_source(ssmd);
        ulm->attach_sink(ssmd);
        ssm->attach_sink(ssmd);

        /* (D) */
        ulm->attach_sink(bbos);
        ssm->attach_sink(bbos);

        /* (E) */
        // sim->add_source(vfin) ?
        ssmd->attach_sink(vfin);

        /* (F) */
        // sim->add_source(kf) ?
        vfin->attach_sink(kf);

        /* (G) */
        kf->attach_sink(kfs);

        /* wconfig=pywebsock.WebserverConfig(7682, False, False, False)
         * web=pywebsock.Webserver.make(wconfig)
         */
        g_ws = Webserver::make(WebserverConfig(7682 /*port*/,
                                               false /*!tls_flag*/,
                                               false /*!strict_host_check_flag*/,
                                               false /*!use_retry_flag*/),
                               PrintJsonSingleton::instance());

        /* web.register_http_endpoint(uls.http_endpoint_descr("/uls")) # /dyn/uls/snap
         * web.register_http_endpoint(kfs.http_endpoint_descr("/kfs")) # /dyn/kfs/snap
         */
        g_ws->register_http_endpoint(uls->http_endpoint_descr(pjson, "/uls"));
        g_ws->register_http_endpoint(kfs->http_endpoint_descr(pjson, "/kfs"));

        /* web.register_stream_endpoint(src.stream_endpoint_descr("/ws/uls"))
         * web.register_stream_endpoint(kf.stream_endpoint_descr("/ws/kfs"))
         */
        g_ws->register_stream_endpoint(src->stream_endpoint_descr("/ws/uls"));
        g_ws->register_stream_endpoint(kf->stream_endpoint_descr("/ws/kfs"));

        log("starting webserver..");

        g_ws->start_webserver();

        log("..webserver started");

        /* attempt simulation.
         * throttle so that we have time to connect browser,  give url
         *   http://localhost:7682/ex_websock.html
         * pywebsock/ex_websock.py implements a governed simulation loop
         * in python,  so that it's interruptible.
         * here, we can rely on simulator instead
         */
        sim->run_throttled_until(t0 /*t1 - ignored if <= sim.t0()*/,
                                 50 /*n_max*/,
                                 2.5 /*replay_factor*/);


        log("joining webserver..");

        g_ws->join_webserver();

        log("..joined webserver");
    } catch (std::exception & ex) {
        std::cerr << "caught exception" << xtag("ex", ex.what()) << std::endl;
    }

} /*main*/

/* end websock_utest_main.cpp */
