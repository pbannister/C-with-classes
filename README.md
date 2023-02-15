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
$ ./build/b2 -h

Usage: 
    ./build/b2 [options]
Where options are:
    -n repeat    : repeat tests              = 100
    -s count     : number of strings         = 100
    -a length    : average string length     = 80
    -q pages     : sample pot size in pages  = 0
    -d seed      : seed for rand()           = 122531496
    -v           : verbose = 0
```

Results are interesting:
```
$ ./build/b2 -n 100 -s 100000 -a 40  

Parameters to test:
   404731931 seed to random
          40 average string length
      100000 strings per array
         100 test runs

==== Using simple samples
  28.796 ns std::string assign from (const char*)
  21.745 ns std::string assign from same
  28.029 ns std::string assign optional from (const char*)
  17.006 ns std::string assign optional from same
  25.462 ns std::string assign [] from (const char*)
  17.657 ns std::string assign [] from same
  45.959 ns base_strings::string_o assign from (const char*)
  19.473 ns base_strings::string_o assign from same
  15.070 ns base_strings::string_o assign [] from (const char*)
  17.698 ns base_strings::string_o assign [] from same

==== Using sample bucket
  33.070 ns std::string assign from (const char*)
  20.893 ns std::string assign from same
  33.438 ns std::string assign optional from (const char*)
  19.171 ns std::string assign optional from same
  33.675 ns std::string assign [] from (const char*)
  21.079 ns std::string assign [] from same
  96.058 ns base_strings::string_o assign from (const char*)
  39.113 ns base_strings::string_o assign from same
  30.571 ns base_strings::string_o assign [] from (const char*)
  37.114 ns base_strings::string_o assign [] from same
```
Seems that **std::string** may have finally caught up, and slightly bettered. 

Playing with the parameters cause the results to vary in ways for which I have no immediate explanation.
(Something wonky around strings in **std::vector**, maybe?)
Not ready to draw conclusion, other than the (horrid) original **std::string** implementation seems to have been replaced with better.

Only took thirty years? :)

