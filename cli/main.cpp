
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <pthread.h>

#include "base/strings.h"
#include "base/threads.h"
#include "base/clocks.h"

using namespace base_strings;
using namespace base_threads;
using namespace base_clocks;

struct options_s {
    int verbose;
} g_options = {
    0  //
};

bool is_verbose(int n = 1) {
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
#if WANT_STRING_SCORECARD
    string_o::scorecard_o o;
    string_o::scorecard_get(o);
    ::printf(
        "1 new : %8u free : %8u\n"
        "2 new : %8u free : %8u\n",
        o.n1_new, o.n1_free,
        o.n2_new, o.n2_free);
#endif
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

    {
        string_o::scorecard_get(sc1);
        string_o a1[20];
        for (auto i = 0; i < 20; ++i) {
            a1[i] = "1999";
            a1[i].strcat(s1);
        }
        string_o::scorecard_get(sc2);
        ASSERT_IF(bump_is(sc1, sc2, 20, 0, 0, 0));
    }
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 20, 20, 0, 0));
    {
        string_o::scorecard_get(sc1);
        string_o a1[20];
        for (auto i = 0; i < 20; ++i) {
            a1[i] = "1999";
            a1[i].strcat(s1);
        }
    }
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 20, 20, 0, 0));

    ::printf("\n==== test 1 - done\n");
    checker.report_score();
    return true;
}

static void* call_test_1(void*) {
    ::printf("\n#### calling test_1\n");
    auto ok = test_1();
    ::printf("\n#### test_1 returns %u\n", ok);
    return 0;
}

bool test_2() {
    ::printf("\n\n=== test_2\n");
    string_o::scorecard_o sc1;
    string_o::scorecard_o sc2;

    string_o::scorecard_get(sc1);
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 0, 0, 0, 0));
    {
        thread_owner_o kid;
        if (!kid.thread_create(call_test_1, (void*) "kid1")) {
            return false;
        }
        ::printf("kid created\n");
        sleep_ms(1000);
    }
    ::printf("kid done?\n");
    sleep_ms(1000);
    string_o::scorecard_get(sc2);
    ASSERT_IF(bump_is(sc1, sc2, 0, 0, 0, 0));

    ::printf("\n\n=== test_2 - done\n");
    return true;
}

int main(int ac, char** av) {
    if (!options_get(ac, av)) {
        return 1;
    }
    if (!test_1()) {
        return 2;
    }
    if (!test_2()) {
        return 2;
    }
    report_scores();
    return 0;
}