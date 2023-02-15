#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "base/strings.h"
#include "base/clocks.h"
#include "samples/samples.h"

#include "version.h"

#include <vector>
#include <string>
#include <optional>

using namespace base_strings;
using namespace base_clocks;

struct options_s {
    int n_seed;
    int n_times;
    int n_strings;
    int n_average;
    int q_sample_pages;
    int verbose;
} g_options = {
    0,
    100,
    100,
    80,
    0,
    0  //
};

bool is_verbose(int n = 1) {
    return n <= g_options.verbose;
}

void usage(const char* av0) {
    ::printf(
        "\nUsage: "
        "\n    %s [options]"
        "\nWhere options are:"
        "\n    -n repeat    : repeat tests              = %u"
        "\n    -s count     : number of strings         = %u"
        "\n    -a length    : average string length     = %u"
        "\n    -q pages     : sample pot size in pages  = %u"
        "\n    -d seed      : seed for rand()           = %u"
        "\n    -v           : verbose (repeat for more) = %u"
        "\n",
        av0,
        g_options.n_times,
        g_options.n_strings,
        g_options.n_average,
        g_options.q_sample_pages,
        g_options.n_seed,
        g_options.verbose);
}

bool options_get(int ac, char** av) {
    g_options.n_seed = base_clocks::clock_realtime::time_get();
    for (;;) {
        int c = ::getopt(ac, av, "d:n:s:a:q:vh");
        if (EOF == c) {
            break;
        }
        switch (c) {
        case 'n':
            g_options.n_times = ::atoi(optarg);
            break;
        case 's':
            g_options.n_strings = ::atoi(optarg);
            break;
        case 'a':
            g_options.n_average = ::atoi(optarg);
            break;
        case 'q':
            g_options.q_sample_pages = ::atoi(optarg);
            break;
        case 'd':
            g_options.n_seed = ::atoi(optarg);
            break;
        case 'v':
            g_options.verbose++;
            break;
        default:
            usage(av[0]);
            return false;
        }
    }
    return true;
}

class benchmark_o {
public:
    simple_sample_o o_sample1;
    bucket_of_samples_o o_sample2;

private:
    const char* picked = 0;

public:
    bool options_apply() {
        ::srand(g_options.n_seed);

        o_sample1.sample_fill(2 * g_options.n_average);
        if (is_verbose(2)) {
            o_sample1.samples_print(100);
        }
        o_sample2.bucket_fill(g_options.n_average, g_options.q_sample_pages);
        if (is_verbose(2)) {
            o_sample2.samples_print(100);
        }

        ::printf(
            "\nParameters to test:\n"
            "%12u seed to random\n"
            "%12u average string length\n"
            "%12u strings per array\n"
            "%12u test runs\n",
            g_options.n_seed,
            g_options.n_average,
            g_options.n_strings,
            g_options.n_times);
        return true;
    }

public:
    struct timed_o {
        timed_ns_t ns_copy_base = 0;
        timed_ns_t ns_copy_same = 0;
        void add(timed_o& o) {
            ns_copy_base += o.ns_copy_base;
            ns_copy_same += o.ns_copy_same;
        }
    };

public:
    void exercise_std1(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        std::vector<std::string> vs1;
        std::vector<std::string> vs2(n_want);
        timed_ns_t ns_samples = 0;
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            ns_samples = o_elapsed.elapsed_ns();
            // Suspect the compiler might optimize this out of existance.
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1.emplace_back(picked);
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - ns_samples;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
    }
    void exercise_std2(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        std::vector<std::string> vs1;
        std::vector<std::optional<std::string>> vs2(n_want);
        timed_ns_t ns_samples = 0;
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            ns_samples = o_elapsed.elapsed_ns();
            // Suspect the compiler might optimize this out of existance.
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1.emplace_back(picked);
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - ns_samples;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
    }
    void exercise_std3(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        std::string vs1[n_want];
        std::string vs2[n_want];
        timed_ns_t ns_samples = 0;
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            ns_samples = o_elapsed.elapsed_ns();
            // Suspect the compiler might optimize this out of existance.
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1[i] = picked;
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - ns_samples;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
    }
    void exercise_base1(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        std::vector<base_strings::string_o> vs1;
        std::vector<base_strings::string_o> vs2(n_want);
        timed_ns_t ns_samples = 0;
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            ns_samples = o_elapsed.elapsed_ns();
            // Suspect the compiler might optimize this out of existance.
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1.emplace_back(picked);
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - ns_samples;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
    }
    void exercise_base2(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        base_strings::string_o vs1[n_want];
        base_strings::string_o vs2[n_want];
        timed_ns_t ns_samples = 0;
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            ns_samples = o_elapsed.elapsed_ns();
            // Suspect the compiler might optimize this out of existance.
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1[i] = picked;
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - ns_samples;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
    }

public:
    bool test_sample(samples_base_o& o_sample, unsigned n_times, unsigned n_strings) {
        {
            timed_o t;
            timed_o sum;
            for (unsigned i = 0; i < n_times; ++i) {
                exercise_std1(t, o_sample, n_strings);
                sum.add(t);
            }
            auto avg1 = (double(sum.ns_copy_base) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign from (const char*)\n", avg1);
            auto avg2 = (double(sum.ns_copy_same) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign from same\n", avg2);
        }
        {
            timed_o t;
            timed_o sum;
            for (unsigned i = 0; i < n_times; ++i) {
                exercise_std2(t, o_sample, n_strings);
                sum.add(t);
            }
            auto avg1 = (double(sum.ns_copy_base) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign optional from (const char*)\n", avg1);
            auto avg2 = (double(sum.ns_copy_same) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign optional from same\n", avg2);
        }
        {
            timed_o t;
            timed_o sum;
            for (unsigned i = 0; i < n_times; ++i) {
                exercise_std3(t, o_sample, n_strings);
                sum.add(t);
            }
            auto avg1 = (double(sum.ns_copy_base) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign [] from (const char*)\n", avg1);
            auto avg2 = (double(sum.ns_copy_same) / n_times) / n_strings;
            ::printf("%8.3f ns std::string assign [] from same\n", avg2);
        }
        {
            timed_o t;
            timed_o sum;
            for (unsigned i = 0; i < n_times; ++i) {
                exercise_base1(t, o_sample, n_strings);
                sum.add(t);
            }
            auto avg1 = (double(sum.ns_copy_base) / n_times) / n_strings;
            ::printf("%8.3f ns base_strings::string_o assign from (const char*)\n", avg1);
            auto avg2 = (double(sum.ns_copy_same) / n_times) / n_strings;
            ::printf("%8.3f ns base_strings::string_o assign from same\n", avg2);
        }
        {
            timed_o t;
            timed_o sum;
            for (unsigned i = 0; i < n_times; ++i) {
                exercise_base2(t, o_sample, n_strings);
                sum.add(t);
            }
            auto avg1 = (double(sum.ns_copy_base) / n_times) / n_strings;
            ::printf("%8.3f ns base_strings::string_o assign [] from (const char*)\n", avg1);
            auto avg2 = (double(sum.ns_copy_same) / n_times) / n_strings;
            ::printf("%8.3f ns base_strings::string_o assign [] from same\n", avg2);
        }
        return true;
    }
};

bool test_run(benchmark_o& benchmark) {
    ::printf("\n==== Using simple samples\n");
    benchmark.test_sample(benchmark.o_sample1, g_options.n_times, g_options.n_strings);
    ::printf("\n==== Using sample bucket\n");
    benchmark.test_sample(benchmark.o_sample2, g_options.n_times, g_options.n_strings);
    return true;
}

int main(int ac, char** av) {
    ::setbuf(stdout, 0);
    ::printf("Version: %s branch: %s build: %u\n", VERSION_STAMP, VERSION_BRANCH, VERSION_BUILD);
    if (!options_get(ac, av)) {
        return 1;
    }
    benchmark_o benchmark;
    if (!benchmark.options_apply()) {
        return 2;
    }
    if (!test_run(benchmark)) {
        return 3;
    }
    return 0;
}
