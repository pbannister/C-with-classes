
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "base/strings.h"

struct options_s {
    int verbose;
} g_options = {
    0  //
};

bool is_verbose(unsigned n = 1) {
    return n <= g_options.verbose;
}

void usage() {
    ::printf(
        "\nUsage: "
        "\n    cli [options]"
        "\nWhere options are:"
        "\n    -v   : verbose = %u"
        "\n",
        g_options.verbose);
}

bool options_get(int ac, char** av) {
    for (;;) {
        int c = ::getopt(ac, av, "vh");
        if (EOF == c) {
            break;
        }
        switch (c) {
        case 'v':
            g_options.verbose++;
            break;
        default:
            usage();
            return false;
        }
    }
    return true;
}

void report_scores() {
    string_o::scorecard_o o;
    string_o::scorecard_get(o);
    ::printf(
        "1 new : %8u free : %8u\n"
        "2 new : %8u free : %8u\n",
        o.n1_new, o.n1_free,
        o.n2_new, o.n2_free);
}

struct scores_o {
    unsigned n_pass = 0;
    unsigned n_fail = 0;
    void assert_if(const char*, int line, bool ok, const char* what) {
        const char* fmt1 = "%s %4d : %s\n";
        if (ok) {
            ++n_pass;
            if (is_verbose(2)) {
                const char* s1 = "PASS";
                ::printf(fmt1, s1, line, what);
            }
        } else {
            ++n_fail;
            if (is_verbose()) {
                const char* s1 = "FAIL";
                ::printf(fmt1, s1, line, what);
            }
        }
    }
    void report_score() {
        ::printf("TOTAL PASS : %8u\n", n_pass);
        ::printf("TOTAL FAIL : %8u\n", n_fail);
    }
};

bool bump_is(
    const string_o::scorecard_o& a,
    const string_o::scorecard_o& b,
    unsigned d1_new, unsigned d1_free, unsigned d2_new, unsigned d2_free) {
    bool ok = true;
    ok &= (d1_new == (b.n1_new - a.n1_new));
    ok &= (d1_free == (b.n1_free - a.n1_free));
    ok &= (d2_new == (b.n2_new - a.n2_new));
    ok &= (d2_free == (b.n2_free - a.n2_free));
    if (is_verbose() && !ok) {
        ::printf(
            "1 new : %8u -> %8u free : %8u -> %8u\n"
            "2 new : %8u -> %8u free : %8u -> %8u\n",
            a.n1_new, b.n1_new, a.n1_free, b.n1_free,
            a.n2_new, b.n2_new, a.n2_free, b.n2_free);
    }
    return ok;
}

scores_o checker;
#define ASSERT_IF(X) checker.assert_if(__FILE__, __LINE__, (X), #X)

bool test_1() {
    ::printf("\n==== test 1\n");
    string_o::scorecard_o sc1;
    string_o::scorecard_o sc2;

    string_o::scorecard_get(sc1);
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 0, 0, 0, 0));

    string_o::scorecard_get(sc1);
    string_o s1;
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 1, 0, 0, 0));

    string_o::scorecard_get(sc1);
    {
        string_o s2;
    }
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 1, 1, 0, 0));

    string_o::scorecard_get(sc1);
    s1.strcpy("123a");
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 0, 0, 0, 0));

    string_o::scorecard_get(sc1);
    s1 = "abc";
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 0, 0, 0, 0));

    ::printf("s1 = `%s`\n", s1.buffer_get());

    string_o::scorecard_get(sc1);
    {
        string_o s3 = s1;
        s3.strcat("1999");
    }
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 1, 1, 0, 0));

    ::printf("s1 = `%s`\n", s1.buffer_get());

    ::printf("\n==== test 1 - done\n");
    checker.report_score();
    return true;
}

bool perform() {
    report_scores();
    for (auto n = 0; n < 10; ++n) {
        string_o s1;
        string_o s2("abc");
        string_o s3;
        s3.strcpy("1234567890123456789012345678901234567890", 7);
        for (auto i = 0; i < 20; ++i) {
            s1.strcpy("123");
            s2.strcat(".");
            s2.strcat(s1);
            s3.strcat("-");
            s3.strcat(s2, 8);
            printf("s1: '%s' s2: '%s' s3: '%s'\n", s1.buffer_get(), s2.buffer_get(), s3.buffer_get());
            report_scores();
        }
    }
    return true;
}

int main(int ac, char** av) {
    if (!options_get(ac, av)) {
        return 1;
    }
    if (!test_1()) {
        return 2;
    }
    report_scores();
    return 0;
}