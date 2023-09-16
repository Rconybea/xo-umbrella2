/* @file ex4.cpp */

#include "indentlog/scope.hpp"

using namespace xo;

class Quadratic {
public:
    Quadratic(double a, double b, double c) : a_{a}, b_{b}, c_{c} {}

    double operator() (double x) const {
        scope log(XO_ENTER0(info), tag("a", a_), xtag("b", b_), xtag("c", c_), xtag("x", x));

        double retval = (a_ * x * x) + (b_ * x) + c_;

        log.end_scope("<-", xtag("retval", retval));

        return retval;
    }

private:
    double a_ = 0.0;;
    double b_ = 0.0;
    double c_ = 0.0;
};

int
main(int argc, char ** argv) {
    //log_config::style  = FS_Pretty;
    log_config::style  = FS_Streamlined;
    log_config::min_log_level = log_level::info;

    scope log(XO_ENTER0(info));

    Quadratic quadratic(2.0, -5.0, 7.0);

    double x = 3.0;
    double r = quadratic(3.0);

    log && log(tag("x", x));
    log && log("<-", xtag("quadratic(x)", r));
}

/* end ex4.cpp */
