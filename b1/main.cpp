#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "base/strings.h"
#include "base/clocks.h"

#include <vector>
#include <string>
#include <optional>

using namespace base_strings;
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
        "\n    b1 [options]"
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

//
//  Adapted/subverted from Daniel Lemire's example:
//  https://lemire.me/blog/2023/01/30/move-or-copy-your-strings-possible-performance-impacts/
//

const static char* short_options[] = {
    "https", "http", "ftp", "file", "ws", "wss", "garbae", "fake", "httpr", "filer",
    0};
const static char* long_options[] = {
    "Let me not to the marriage of true minds",
    "Love is not love Which alters when it alteration finds",
    " And every fair from fair sometimes declines",
    "Wisely and slow; they stumble that run fast.",
    "Adapted/subverted from Daniel Lemire's example:",
    "https://lemire.me/blog/2023/01/30/move-or-copy-your-strings-possible-performance-impacts/",
    "Love is not love Which alters when it alteration finds And every fair from fair sometimes declines",
    "Let me not to the marriage of true minds Love is not love Which alters when it alteration finds",
    0};

struct samples_o {
    unsigned n_samples = 0;
    const char** a_samples = 0;
    void samples_set(const char** pp) {
        a_samples = pp;
        for (n_samples = 0; *pp; ++pp) {
            ++n_samples;
        }
    }
    const char* sample_pick() const {
        unsigned i = (rand() % n_samples);
        return a_samples[i];
    }
    samples_o(const char** pp) {
        samples_set(pp);
    }
};

void populate1(std::vector<std::string>& answer, const samples_o& o_samples, unsigned n_want) {
    answer.reserve(n_want);
    for (unsigned n = 0; n < n_want; ++n) {
        const char* picked = o_samples.sample_pick();
        answer.emplace_back(picked);
    }
}

timed_ns_t simulation1(const samples_o& o_samples, unsigned n_want) {
    std::vector<std::string> vs1;
    std::vector<std::optional<std::string>> vs2(n_want);
    populate1(vs1, o_samples, n_want);
    elapsed_o o_elapsed;
    for (unsigned i = 0; i < n_want; ++i) {
        vs2[i] = vs1[i];
    }
    o_elapsed.clock_split();
    return o_elapsed.elapsed_ns();
}

typedef std::vector<string_o> vector_of_strings_o;

void populate2(vector_of_strings_o& answer, const samples_o& o_samples, unsigned n_want) {
    answer.reserve(n_want);
    for (unsigned n = 0; n < n_want; ++n) {
        const char* picked = o_samples.sample_pick();
        answer.emplace_back(picked);
    }
}

timed_ns_t simulation2(const samples_o& o_samples, unsigned n_want) {
    vector_of_strings_o vs1;
    vector_of_strings_o vs2(n_want);
    populate2(vs1, o_samples, n_want);
    elapsed_o o_elapsed;
    for (unsigned i = 0; i < n_want; ++i) {
        vs2[i] = vs1[i];
    }
    o_elapsed.clock_split();
    return o_elapsed.elapsed_ns();
}

bool test_1() {
    samples_o o_samples_short(short_options);
    samples_o o_samples_long(long_options);
    constexpr unsigned n_times = 100;
    constexpr unsigned n_size = 131072;
    // constexpr unsigned n_size = 10;
    {
        timed_ns_t ns1 = 0;
        ::srand(20220211);
        for (unsigned i = 0; i < n_times; ++i) {
            ns1 += simulation1(o_samples_short, n_size);
        }
        timed_ns_t ns2 = 0;
        for (unsigned i = 0; i < n_times; ++i) {
            ns2 += simulation1(o_samples_long, n_size);
        }
        auto avg1 = (double(ns1) / n_times) / n_size;
        auto avg2 = (double(ns2) / n_times) / n_size;
        ::printf("%8.3f ns std::string copy short\n", avg1);
        ::printf("%8.3f ns std::string copy long\n", avg2);
    }
    {
        timed_ns_t ns1 = 0;
        ::srand(20220211);
        for (unsigned i = 0; i < n_times; ++i) {
            ns1 += simulation2(o_samples_short, n_size);
        }
        timed_ns_t ns2 = 0;
        for (unsigned i = 0; i < n_times; ++i) {
            ns2 += simulation2(o_samples_long, n_size);
        }
        auto avg1 = (double(ns1) / n_times) / n_size;
        auto avg2 = (double(ns2) / n_times) / n_size;
        ::printf("%8.3f ns string copy short\n", avg1);
        ::printf("%8.3f ns string copy long\n", avg2);
    }
    {
        timed_ns_t ns1 = 0;
        ::srand(20220211);
        for (unsigned i = 0; i < n_times; ++i) {
            ns1 += simulation2(o_samples_short, n_size);
        }
        timed_ns_t ns2 = 0;
        for (unsigned i = 0; i < n_times; ++i) {
            ns2 += simulation2(o_samples_long, n_size);
        }
        auto avg1 = (double(ns1) / n_times) / n_size;
        auto avg2 = (double(ns2) / n_times) / n_size;
        ::printf("%8.3f ns string copy short\n", avg1);
        ::printf("%8.3f ns string copy long\n", avg2);
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
    return 0;
}
