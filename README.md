# C-with-classes and strings
Motive from Daniel Lemire's post:<br>
https://lemire.me/blog/2023/01/30/move-or-copy-your-strings-possible-performance-impacts/

I have long used C-with-classes style strings, as measurably more efficient:<br>
https://bannister.us/weblog/2005/building-a-better-string-class

Used variants of this approach since the early 1990s. 
But the last time I did a benchmark was 2005, so perhaps things have changed. 
Started with Daniel's benchmark, then changed pretty much everything.

Should add that I had before never needed a multi-thread safe strings class. 
(Last large code in C++ was early 2000s, and single-threaded.)
Took this as excuse to make use of **thread_local**, so now I have a thread-safe C-with-classes string class.

Using CMake for the build, and VScode for editing. Once built, "-h" will give usage:
```
$ ./build/bin/b2  -h
Version: 2026.09.15 branch: master build: 69

Usage: 
    ./build/bin/b2 [options]
Where options are:
    -t repeat    : repeat time (seconds)     = 1
    -s count     : number of strings         = 100
    -a length    : average string length     = 40
    -q pages     : sample pot size in pages  = 0
    -d seed      : seed for rand()           = 2481292659
    -v           : verbose (repeat for more) = 0```
```

Results are interesting:
```
$ ./build/bin/b2 
Version: 2026.09.15 branch: master build: 69

Parameters to test:
   949220720 seed to random
          40 average string length
         100 strings per array
           3 test time (seconds)

==== Once through both
Warmup : 10000 iterations in    169 ms -- sample1
Warmup : 10000 iterations in    182 ms -- sample2
Use    : 60000 iterations for     3 seconds

==== Using simple samples
std::string    6.959 ns : base_strings::string_local_o    6.792 ns pick
std::string    0.931 ns : base_strings::string_local_o    4.046 ns instantiate
std::string   31.820 ns : base_strings::string_local_o   16.097 ns assign from (const char*)
std::string   19.539 ns : base_strings::string_local_o   15.172 ns assign from same
std::string    7.219 ns : base_strings::string_local_o    2.781 ns dispose

==== Using sample bucket
std::string    6.930 ns : base_strings::string_local_o    6.177 ns pick
std::string    1.021 ns : base_strings::string_local_o    3.901 ns instantiate
std::string   29.588 ns : base_strings::string_local_o   18.542 ns assign from (const char*)
std::string   22.125 ns : base_strings::string_local_o   16.509 ns assign from same
std::string    8.112 ns : base_strings::string_local_o    2.804 ns dispose
```

Seems that **std::string** may have finally caught up. 
Only took thirty years? :)

First iteration was buggy, as I did not know that thread ID was address-based and not unique over time within the same process.
(This seems to me a very poor design choice, but that is now fixed history.)

First results varied in ways I could not explain.

At first, mirrored Lemire's use of **std::vector**.
Wanted to be sure of behavior, so switched to plain arrays.

In the present, I make use of AI (in the form of LLM - Large Language Model) in programming.
Asked the LLM to inspect the code, and as usual it did some astonishing deep analysis,
and came to some wild (and incorrect) conclusions.
Filtered for what looked reasonable, and folded in some improvements.

So there are some LLM-inspired optimizations in the code.

After a few iterations, the picture became clearer.
Turns out, **glibc** has some built-in behaviors that mucked up the results
(see `RESULTS.md` for details near the end).

To be clear, this is not meant as a general-purpose string class. 
Rather this is meant to be an efficient string class for use within my applications.

