#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "base/strings.h"
#include "base/clocks.h"
#include "samples/samples.h"

#include "version.h"

#include <string>

using namespace base_strings;
using namespace base_clocks;

struct options_s {
    int n_seed;
    int n_seconds;
    int n_strings;
    int n_average;
    int q_sample_pages;
    int verbose;
} g_options = {
    0,
    1,
    100,
    40,
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
        "\n    -t repeat    : repeat time (seconds)     = %u"
        "\n    -s count     : number of strings         = %u"
        "\n    -a length    : average string length     = %u"
        "\n    -q pages     : sample pot size in pages  = %u"
        "\n    -d seed      : seed for rand()           = %u"
        "\n    -v           : verbose (repeat for more) = %u"
        "\n",
        av0,
        g_options.n_seconds,
        g_options.n_strings,
        g_options.n_average,
        g_options.q_sample_pages,
        g_options.n_seed,
        g_options.verbose);
}

bool options_get(int ac, char** av) {
    g_options.n_seed = base_clocks::clock_realtime::time_get();
    for (;;) {
        int c = ::getopt(ac, av, "d:t:s:a:q:vh");
        if (EOF == c) {
            break;
        }
        switch (c) {
        case 't':
            g_options.n_seconds = ::atoi(optarg);
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
    samples_simple_o o_sample1;
    samples_bucket_o o_sample2;

private:
    const char* picked = 0;

public:
    bool options_apply() {
        ::srand(g_options.n_seed);

        o_sample1.sample_fill(2 * g_options.n_average);
        if (is_verbose(2)) {
            o_sample1.samples_print("sample1", 100);
        }
        o_sample2.bucket_fill(g_options.n_average, g_options.q_sample_pages);
        if (is_verbose(2)) {
            o_sample2.samples_print("sample2", 100);
        }

        ::printf(
            "\nParameters to test:\n"
            "%12u seed to random\n"
            "%12u average string length\n"
            "%12u strings per array\n"
            "%12u test time (seconds)\n",
            g_options.n_seed,
            g_options.n_average,
            g_options.n_strings,
            g_options.n_seconds);
        return true;
    }

public:
    struct timed_o {
        timed_ns_t ns_instantiate = 0;
        timed_ns_t ns_copy_base = 0;
        timed_ns_t ns_copy_same = 0;
        timed_ns_t ns_dispose = 0;
        timed_ns_t ns_pick = 0;
        void add(timed_o& o) {
            ns_instantiate += o.ns_instantiate;
            ns_copy_base += o.ns_copy_base;
            ns_copy_same += o.ns_copy_same;
            ns_dispose += o.ns_dispose;
            ns_pick += o.ns_pick;
        }
    };

    static double ns_average(timed_ns_t ns, unsigned n_times, unsigned n_objects) {
        return (double(ns) / n_times) / n_objects;
    }

    void report_times(unsigned n_times, unsigned n_strings, const timed_o& sum1, const char* what1, const timed_o& sum2, const char* what2) {
        // Instantiate and dispose each cover two arrays of n_strings.
        const unsigned n_objects = 2 * n_strings;
        auto ns_pick1 = ns_average(sum1.ns_pick, n_times, n_strings);
        auto ns_pick2 = ns_average(sum2.ns_pick, n_times, n_strings);
        auto ns_instantiate1 = ns_average(sum1.ns_instantiate, n_times, n_objects);
        auto ns_instantiate2 = ns_average(sum2.ns_instantiate, n_times, n_objects);
        auto ns_copy_base1 = ns_average(sum1.ns_copy_base, n_times, n_strings);
        auto ns_copy_base2 = ns_average(sum2.ns_copy_base, n_times, n_strings);
        auto ns_copy_same1 = ns_average(sum1.ns_copy_same, n_times, n_strings);
        auto ns_copy_same2 = ns_average(sum2.ns_copy_same, n_times, n_strings);
        auto ns_dispose1 = ns_average(sum1.ns_dispose, n_times, n_objects);
        auto ns_dispose2 = ns_average(sum2.ns_dispose, n_times, n_objects);
        ::printf("%s %8.3f ns : %s %8.3f ns pick\n",  //
                 what1, ns_pick1, what2, ns_pick2);
        ::printf("%s %8.3f ns : %s %8.3f ns instantiate\n",  //
                 what1, ns_instantiate1, what2, ns_instantiate2);
        ::printf("%s %8.3f ns : %s %8.3f ns assign from (const char*)\n",  //
                 what1, ns_copy_base1, what2, ns_copy_base2);
        ::printf("%s %8.3f ns : %s %8.3f ns assign from same\n",  //
                 what1, ns_copy_same1, what2, ns_copy_same2);
        ::printf("%s %8.3f ns : %s %8.3f ns dispose\n",  //
                 what1, ns_dispose1, what2, ns_dispose2);
    }

public:
    //
    //  Arrays, not vectors: one allocation, no growth or reallocation,
    //  so what is measured is the string class.  The arrays live on the
    //  heap, as two std::string arrays of -s 200000 overflow an 8 MB stack.
    //
    template <typename T>
    void exercise_one(timed_o& t, const samples_base_o& o_samples, unsigned n_want) {
        T* vs1 = 0;
        T* vs2 = 0;
        {
            // Warm up the cache and the allocator.
            vs1 = new T[n_want];
            vs2 = new T[n_want];
            delete[] vs1;
            delete[] vs2;
        }
        {
            elapsed_o o_elapsed;
            vs1 = new T[n_want];
            vs2 = new T[n_want];
            o_elapsed.clock_split();
            t.ns_instantiate = o_elapsed.elapsed_ns();
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
            }
            o_elapsed.clock_split();
            t.ns_pick = o_elapsed.elapsed_ns();
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                picked = o_samples.sample_pick();
                vs1[i] = picked;
            }
            o_elapsed.clock_split();
            t.ns_copy_base = o_elapsed.elapsed_ns() - t.ns_pick;
        }
        {
            elapsed_o o_elapsed;
            for (unsigned i = 0; i < n_want; ++i) {
                vs2[i] = vs1[i];
            }
            o_elapsed.clock_split();
            t.ns_copy_same = o_elapsed.elapsed_ns();
        }
        {
            elapsed_o o_elapsed;
            delete[] vs1;
            delete[] vs2;
            o_elapsed.clock_split();
            t.ns_dispose = o_elapsed.elapsed_ns();
        }
    }

public:
    // Do a n_seconds warmup of the sample to get the allocator and cache warmed up.
    // Return the number of iterations performed.
    unsigned sample_warm(samples_base_o& o_sample, timed_ms_t& ms_want, unsigned n_strings) {
        timed_o t;
        elapsed_o o_elapsed;
        unsigned n_times = 10000;
        for (unsigned i = 0; i < n_times; ++i) {
            exercise_one<std::string>(t, o_sample, n_strings);
            exercise_one<string_local_o>(t, o_sample, n_strings);
            if (0 == (i % 100)) {
                o_elapsed.clock_split();
                auto ms_now = o_elapsed.elapsed_ms();
                if (ms_want <= ms_now) {
                    n_times = i + 1;
                    break;
                }
            }
        }
        o_elapsed.clock_split();
        ms_want = o_elapsed.elapsed_ms();
        return n_times;
    }

public:
    bool sample_test(samples_base_o& o_sample, unsigned n_times, unsigned n_strings) {
        timed_o sum1;
        timed_o sum2;
        for (unsigned i = 0; i < n_times; ++i) {
            timed_o t;
            exercise_one<std::string>(t, o_sample, n_strings);
            sum1.add(t);
        }
        for (unsigned i = 0; i < n_times; ++i) {
            timed_o t;
            exercise_one<string_local_o>(t, o_sample, n_strings);
            sum2.add(t);
        }
        report_times(n_times, n_strings, sum1, "std::string", sum2, "base_strings::string_local_o");
        return true;
    }
};

bool test_run(benchmark_o& benchmark) {
    ::printf("\n==== Once through both\n");
    auto ms_warm_wanted = 1000;            // 1 second in milliseconds
    timed_ms_t ms_warm1 = ms_warm_wanted;  // 1 second in milliseconds
    timed_ms_t ms_warm2 = ms_warm_wanted;  // 1 second in milliseconds
    auto n_times1 = benchmark.sample_warm(benchmark.o_sample1, ms_warm1, g_options.n_strings);
    auto n_times2 = benchmark.sample_warm(benchmark.o_sample2, ms_warm2, g_options.n_strings);
    auto n_times = (((n_times1 + n_times2) * g_options.n_seconds * 1000) / ms_warm_wanted);
    ::printf("Warmup : %5u iterations in  %5lu ms -- sample1\n", n_times1, ms_warm1);
    ::printf("Warmup : %5u iterations in  %5lu ms -- sample2\n", n_times2, ms_warm2);
    ::printf("Use    : %5u iterations for %5u seconds\n", n_times, g_options.n_seconds);
    ::printf("\n==== Using simple samples\n");
    benchmark.sample_test(benchmark.o_sample1, n_times, g_options.n_strings);
    ::printf("\n==== Using sample bucket\n");
    benchmark.sample_test(benchmark.o_sample2, n_times, g_options.n_strings);
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
