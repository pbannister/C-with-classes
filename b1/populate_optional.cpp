
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <string_view>

uint64_t nano() {
    return std::chrono::duration_cast<::std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

template <bool shortstrings>
std::vector<std::string> populate(size_t length) {
    // ::srand(20230211);  // Always want same sequence.
    const static char *short_options[] = {"https", "http", "ftp", "file", "ws", "wss", "garbae", "fake", "httpr", "filer"};
    const static char *long_options[] = {
        "Let me not to the marriage of true minds",
        "Love is not love Which alters when it alteration finds",
        " And every fair from fair sometimes declines",
        "Wisely and slow; they stumble that run fast."};
    std::vector<std::string> answer;
    answer.reserve(length);
    for (size_t pos = 0; pos < length; pos++) {
        const char *picked = shortstrings
                                 ? short_options[rand() % std::size(short_options)]
                                 : long_options[rand() % std::size(long_options)];
        answer.emplace_back(picked);
    }
    return answer;
}

struct times_o {
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    void add(const times_o &o) {
        t1 += o.t1;
        t2 += o.t2;
        t3 += o.t3;
        t4 += o.t4;
    }
    void divide(unsigned n) {
        t1 /= n;
        t2 /= n;
        t3 /= n;
        t4 /= n;
    }
};

template <bool shortstrings>
void simulation(times_o &ts, const size_t N) {
    std::vector<std::string> data;
    data = populate<shortstrings>(N);
    {
        std::vector<std::optional<std::string>> outdata;
        outdata = std::vector<std::optional<std::string>>(N);

        uint64_t start = nano();
        for (size_t i = 0; i < N; i++) {
            outdata[i] = data[i];
        }
        uint64_t finish = nano();
        ts.t1 = double(finish - start) / N;
    }
    {
        std::vector<std::optional<std::string>> outdata;
        outdata = std::vector<std::optional<std::string>>(N);

        uint64_t start = nano();
        for (size_t i = 0; i < N; i++) {
            outdata[i] = std::move(data[i]);
        }
        uint64_t finish = nano();
        ts.t2 = double(finish - start) / N;
    }
    {
        std::vector<std::string *> outdataptr;
        outdataptr = std::vector<std::string *>(N);

        uint64_t start = nano();
        for (size_t i = 0; i < N; i++) {
            outdataptr[i] = &data[i];
        }
        uint64_t finish = nano();
        ts.t3 = double(finish - start) / N;
    }
    {
        std::vector<std::optional<std::string_view>> outdata_view;
        outdata_view = std::vector<std::optional<std::string_view>>(N);

        uint64_t start = nano();
        for (size_t i = 0; i < N; i++) {
            outdata_view[i] = std::string_view(data[i]);
        }
        uint64_t finish = nano();
        ts.t4 = double(finish - start) / N;
    }
}

template <bool shortstrings>
void demo() {
    times_o avg;
    size_t times = 100;

    for (size_t i = 0; i < times; i++) {
        times_o ts;
        simulation<shortstrings>(ts, 131072);
        avg.add(ts);
    }

    avg.divide(times);

    std::cout << avg.t1 << " " << avg.t2 << " " << avg.t3 << " " << avg.t4 << std::endl;
}

int main() {
    puts("We report ns/string (first copy, then move, then pointer, the string_view).\n");
    puts("short strings:");
    demo<true>();
    puts("long strings:");
    demo<false>();
}