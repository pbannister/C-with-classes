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
$ ./build/bin/b2 -h
Version: 2023.02.15 branch: master build: 3

Usage: 
    ./build/bin/b2 [options]
Where options are:
    -n repeat    : repeat tests              = 100
    -s count     : number of strings         = 100
    -a length    : average string length     = 80
    -q pages     : sample pot size in pages  = 0
    -d seed      : seed for rand()           = 4179899174
    -v           : verbose (repeat for more) = 0
```

Results are interesting:
```
$ ./build/bin/b2 -n 100 -s 100000 -a 40
Version: 2023.02.15 branch: master build: 3

Parameters to test:
  2257230900 seed to random
          40 average string length
      100000 strings per array
         100 test runs

==== Using simple samples
  26.773 ns std::string assign from (const char*)
  18.650 ns std::string assign from same
  28.968 ns std::string assign optional from (const char*)
  20.660 ns std::string assign optional from same
  26.737 ns std::string assign [] from (const char*)
  19.019 ns std::string assign [] from same
  49.826 ns base_strings::string_o assign from (const char*)
  20.617 ns base_strings::string_o assign from same
  16.434 ns base_strings::string_o assign [] from (const char*)
  19.886 ns base_strings::string_o assign [] from same

==== Using sample bucket
  34.824 ns std::string assign from (const char*)
  21.596 ns std::string assign from same
  34.373 ns std::string assign optional from (const char*)
  19.474 ns std::string assign optional from same
  33.416 ns std::string assign [] from (const char*)
  20.518 ns std::string assign [] from same
  96.127 ns base_strings::string_o assign from (const char*)
  37.021 ns base_strings::string_o assign from same
  29.484 ns base_strings::string_o assign [] from (const char*)
  33.535 ns base_strings::string_o assign [] from same
```
Seems that **std::string** may have finally caught up, and slightly bettered. 

Playing with the parameters cause the results to vary in ways for which I have no immediate explanation.
(Something wonky around strings in **std::vector**, maybe?)
Not ready to draw conclusion, other than the (horrid) original **std::string** implementation seems to have been replaced with better.

Only took thirty years? :)

