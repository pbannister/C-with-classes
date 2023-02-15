#include "samples.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern bool is_verbose(int n = 1);

// Limit bucket size.
static constexpr unsigned n_size_max = (1 << 30);
static constexpr unsigned n_size_page = (1 << 12);

static inline unsigned limit_to(unsigned v, unsigned v_max) {
    return ((v <= v_max) ? v : v_max);
}

void samples_base_o::samples_print(unsigned n) {
    unsigned t = 0;
    for (unsigned i = 0; i < n; ++i) {
        auto s = sample_pick();
        t += ::strlen(s);
        if (is_verbose(3)) {
            ::printf("Sample: '%s'\n", s);
        }
    }
    auto av = ((t * 1.0) / n);
    ::printf("Samples %u average: %8.2f\n", n, av);
}

//
//  Implementation for bucket-of-samples.
//

bucket_of_samples_o::~bucket_of_samples_o() {
    delete p_bucket;
    p_bucket = 0;
    n_space = 0;
}

const char* bucket_of_samples_o::sample_pick() const {
    return p_bucket + (::rand() % n_space);
}

//
//  Generate a distribution of string averaging length n_average.
//

bool bucket_of_samples_o::bucket_fill(unsigned n_average, unsigned q) {
    delete p_bucket;  // In case not first call.
    p_bucket = 0;
    // Translate subindex to printable character.
    char xlat[256];
    {
        for (unsigned i = 0; i < 256; ++i) {
            char c = ' ' + (i % 95);
            xlat[i] = c;
        }
    }
    // Size the bucket to whole pages.
    n_space = limit_to((n_size_page << q), n_size_max);
    if (is_verbose()) {
        ::printf("bucket space: %u\n", n_space);
    }
    p_bucket = new char[n_space];
    // Fill bucket with sequential characters.
    for (unsigned i = 0; i < n_space; ++i) {
        char c = xlat[255 & i];
        p_bucket[i] = c;
    }
    p_bucket[n_space - 1] = 0;
    // Insert EOS to chop into strings.
    unsigned n_wanted = (n_space / (n_average + 1));
    for (unsigned n = 0; n < n_wanted;) {
        unsigned i = (::rand() % n_space);
        if (p_bucket[i]) {
            ++n;
            p_bucket[i] = 0;
        }
    }
    if (0 && is_verbose(2)) {
        unsigned a_length[n_space] = {0};
        unsigned n_strings = 0;
        unsigned n_total = 0;
        {
            auto p1 = p_bucket;
            auto p2 = p1 + n_space;
            for (; p1 < p2;) {
                ++n_strings;
                unsigned n_length = ::strlen(p1);
                a_length[n_length]++;
                // ::printf("%4u %4u '%s'\n", n_strings, n_length, p1);
                n_total += n_length;
                p1 = ::strchr(p1, 0) + 1;
            }
        }
        for (unsigned n = 0; n < n_space; ++n) {
            auto ns = a_length[n];
            if (ns) {
                ::printf("%8u : %6u\n", n, ns);
            }
        }
        {
            float av = n_total;
            av /= n_strings;
            ::printf("\n==== Strings: %u average: %8.2f\n", n_strings, av);
        }
    }
    return true;
}

//
//  Implementation for simple sample (substrings-from-one-string).
//

bool simple_sample_o::sample_fill(unsigned n) {
    // Translate subindex to printable character.
    char xlat[256];
    {
        for (unsigned i = 0; i < 256; ++i) {
            char c = ' ' + (i % 95);
            xlat[i] = c;
        }
    }
    // Fill buffer with sequence.
    char* p = new char[n + 1];
    for (unsigned i = 0; i < n; ++i) {
        char c = xlat[255 & i];
        p[i] = c;
    }
    p[n] = 0;
    delete p_sample;
    p_sample = p;
    n_sample = n;
    return false;
}

const char* simple_sample_o::sample_pick() const {
    unsigned i = (::rand() % n_sample);
    return p_sample + i;
}

simple_sample_o::~simple_sample_o() {
    delete p_sample;
    p_sample = 0;
    n_sample = 0;
}
