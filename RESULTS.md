# RESULTS — C-with-classes `string_local_o` and the b2 benchmark

Record of findings from a review/rework session. Everything below is measured on this
machine unless noted: Ubuntu/glibc 2.39, x86-64, `-O3`, L1d 32 KB / 64 B line,
L2 512 KB, L3 32 MB, 8 MB stack.

Code state: commit `3fbd72d` ("Working thread-local strings in C-with-classes style.").

---

## 1. Correctness findings

Notes on ownership semantics as they now stand:

- The main thread's recycler deliberately **does not free its pages** at exit, because
  namespace-scope objects with string members are destroyed after the main thread's
  `thread_local`. Demonstrated: a global `string_local_o` is destroyed after the
  recycler, yet stays valid and frees back into the retained pool.
- Worker threads free their pages at exit; `id_thread = 0` marks a retired recycler.
- The pool never shrinks: peak live strings x 128 B per thread is retained for the life
  of that thread. That is the design's footprint cost, not a leak in the usual sense.

---

## 2. Benchmark methodology findings

*  **glibc's `M_TRIM_THRESHOLD` (default 128 KB) contaminates `instantiate` at large `-s`.**
   When the *object array* is freed and it is at the top of the heap, glibc returns those
   pages to the kernel; the next iteration's `new` faults them back in, inside the timed
   region. The object array is `32 x n` bytes for `std::string`, so this starts at
   `-s ≈ 4096`:

   ```
   -s       1000    2000    4000    8000
   std default       0.603   0.586   5.681   8.246   ns/object
   std trim pinned   0.597   0.585   0.584   0.651   ns/object   (flat)
   ```

   Pinning `MALLOC_MMAP_THRESHOLD_` alone changes nothing; the levers are
   `MALLOC_TRIM_THRESHOLD_` and `MALLOC_TOP_PAD_`. 
---

## 3. Performance in the intended regime

Intended use: cyclic application, **hundreds** of live strings, strings mostly
< 128 bytes, most text handled as pointers into one large buffer rather than as live
string objects. In that regime the allocator artifacts above do not appear, and the two
sample blocks agree within noise:

`b2 -a 40 -t 1`, `simple`/`bucket`, ns per object:

```
-s 100    std:  inst 0.98/0.96   asgn 32.1/29.8   same 19.0/21.0   disp  7.3/8.2
          base: inst 4.62/4.28   asgn 16.4/20.8   same 15.3/18.6   disp  2.85/3.09
-s 500    base: inst 3.32/2.95   asgn 16.4/17.9   same 16.7/16.1   disp  2.22/2.15
-s 1000   base: inst 3.13/3.50   asgn 14.5/20.5   same 15.0/19.6   disp  1.94/2.93
```

Summary for that regime:

| phase | `std::string` | `string_local_o` | winner |
|---|---|---|---|
| instantiate | 0.6–1.0 ns | 3.1–4.6 ns | std — empty/SSO vs eager 128-byte chunk |
| assign from `const char*` | 28–32 ns | 14.5–20.8 ns | **base ~2x** |
| assign from same | 17–21 ns | 15–19 ns | base slightly |
| dispose | 7–15 ns | 1.9–3.1 ns | **base 3–5x** |

Per cycle (create + assign + dispose) that is roughly **20 ns for `string_local_o` vs
38 ns for `std::string`** at `-a 40`. The class wins the phases that touch the data and
loses only bare instantiation, which is the eager allocation.

Length-band detail (separate reuse/churn microbenchmark, `-O3`, ns/op):

```
churn (create / use / dispose per cycle)
   <= 15 B :  std 8.3   base 12.6    <- std SSO wins
  16–127 B :  std 23.0  base 13.0    <- pool wins ~1.8x
   > 127 B :  std 29.5  base 27.0    <- tie (both malloc)
reuse (long-lived strings assigned repeatedly)
   <= 15 B :  tie
  16–127 B :  base ~10% ahead
   > 127 B :  base ahead
```

The crossover is `std::string`'s SSO threshold (~15 bytes).

### Size classes: measured

Measured with the present benchmark
(`-s 20000 -t 1`, simple samples, assign + dispose per string, steady state):

| shape | `-a 80` | `-a 200` | `-a 400` |
|---|---|---|---|
| single 128 B class (current) | 22.6 + 6.3 = 28.9 | 32.4 + 17.5 = 49.9 | 50.9 + 40.4 = 91.3 |
| 128 B + 1 KB (coarse) | 19.1 + 3.7 = 22.8 | 40.0 + 8.0 = 48.0 | 84.9 + 9.9 = 94.8 |
| geometric ladder 128→4096 | 20.8 + 4.5 = 25.3 | 32.5 + 7.7 = 40.2 | 63.5 + 9.6 = 73.1 |

- The **ladder is consistently 10–20% better** in steady state: `dispose` drops sharply
  (no `free`) and `assign` no longer calls `malloc`.
- The **coarse 1 KB class is mixed** — best at `-a 80`, worst at `-a 400`; its internal
  fragmentation (1 KB for a 200-byte string) costs cache.
- If classes are added, use a tight geometric ladder, and give each class an arena list
  the destructor can free — the measured variants leaked the extra-class arenas.

Conclusion: **the single class is not clearly optimal.** 
For the intended workload (strings mostly < 128 B, hundreds live) 
a ladder would rarely be exercised beyond the first one or
two classes, so the case for adding it is weaker than these `-s 20000` numbers suggest.

---

## 4. Design conclusions

1. The thread-local, single-class pool is a good fit for the stated workload. Cyclic
   create/use/dispose with hundreds of live strings stays L2-resident
   (100–1000 x 128 B = 13–128 KB) and the LIFO free list reuses hot chunks; cost is
   ~13 ns/string against ~23 ns for `std::string` in the 16–127 byte band.
2. A size-class **ladder** shows a repeatable 10–20% steady-state win when re-measured
   (§3); a *coarse* class is mixed. The earlier "do not add classes / keep one class"
   conclusion came from an under-warmed harness and should not be relied on. Adding a
   ladder is defensible if medium strings (128–4096 B) are common; for the intended
   workload it would rarely be touched, so it remains optional.
3. The pool's real weakness is **bare instantiation** versus SSO: a default
   `string_local_o` eagerly takes a 128-byte chunk. If the application creates strings it
   does not immediately fill, or if most strings are <= 15 bytes, `std::string`'s inline
   storage wins that cycle (8.3 vs 12.6 ns). If that matters, **SSO (a small inline
   buffer)** is the change that addresses it — size classes do not, since every string
   still pays a pool round trip. Lazy allocation is the cheaper alternative but does not
   help the churn case either (the assignment still pays the pool round trip).
4. The 128-byte minimum costs **memory**, not time, in the intended regime. It only
   becomes a timing effect (L2/L3 crossing, instantiate climbing 3.1 → 10.5 ns) when live
   strings reach the thousands — which the intended use explicitly excludes.
5. The pool never returns memory. For a long-lived thread pool that is the price; for a
   bursty process it means peak-live footprint is retained. A high-water trim is the
   lever if that ever matters.
6. Benchmark hygiene for this purpose: keep `-s` in the hundreds; measure `instantiate`
   once (it is sample-independent) rather than per sample block/size; compare std/base
   only within a block; do not compare `instantiate` across sizes.

---

## 5. Reproduce

```sh
# build (mkversion.sh bumps version.h; restore it if you care)
cmake -S . -B build && cmake --build build -j4

# correctness
./build/bin/cli                 # expect 0 pass / 0 fail with scorecard off

# intended regime
./build/bin/b2 -s 100 -a 40 -t 1
./build/bin/b2 -s 500 -a 40 -t 1

# glibc trim artifact (appears at -s >= ~4096 for std::string)
./build/bin/b2 -s 8000 -a 40 -t 1
MALLOC_TRIM_THRESHOLD_=1073741824 MALLOC_TOP_PAD_=1073741824 ./build/bin/b2 -s 8000 -a 40 -t 1

# sanitizers
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -I. cli/main.cpp base/strings.cpp base/threads.cpp -o /tmp/cli-asan -lpthread
g++ -std=c++17 -O1 -g -fsanitize=thread -I. cli/main.cpp base/strings.cpp base/threads.cpp -o /tmp/cli-tsan -lpthread
setarch $(uname -m) -R /tmp/cli-tsan     # TSan needs ASLR disabled here
```

Aliasing repro (expect correct output, and ASan-clean):

```cpp
string_local_o s; s = "hello";                       // 'hello'
string_local_o t; t.strcpy("world");                 // 'world'
string_local_o u("abc"); u.strcat("def");            // 'abcdef'
string_local_o v; for (int i=0;i<40;i++) v.strcat("0123456789");  // len 400
// s = s;  s.strcpy(s.buffer_get());  s.strcat(s.buffer_get());  -> throw exception_o
//         (uncaught here, so it terminates; that is the intended misuse diagnostic)
```
