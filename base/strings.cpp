#include "strings.h"
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>

extern bool is_verbose(int n = 1);

using namespace base_strings;

//
//  Strings of small/normal size are recycled for this string class.
//  Note this allocates small strings from pages, so less heap traffic.
//

struct string_recycler_o {
    const unsigned n_size_page = (1 << 12);
    const unsigned n_size_small = (1 << 7);

    struct recycled_buffer_o;
    typedef recycled_buffer_o* recycled_buffer_p;
    struct recycled_buffer_o {
        recycled_buffer_p p_next;
    };

    recycled_buffer_p p_free = 0;
    string_o::scorecard_o scores;  // for debugging

    void free_print();
    void page_add();

    char* buffer_new();
    char* buffer_new(unsigned n);
    void buffer_free(char* s, unsigned n);

    char* buffer_needed(unsigned n) {
        return ((n_size_small <= n) ? buffer_new(n) : buffer_new());
    }

    string_recycler_o() {
        page_add();
    }
    ~string_recycler_o();
};

#if WANT_STRING_SCORECARD
#define X_TRACE(L)          \
    {                       \
        if (is_verbose()) { \
            ::printf L;     \
        }                   \
    }
#define N1_NEW() \
    { scores.n1_free++; }
#define N1_FREE() \
    { scores.n1_free++; }
#define N2_NEW() \
    { scores.n2_free++; }
#define N2_FREE() \
    { scores.n2_free++; }
#else
#define X_TRACE(L) \
    {}
#define N1_NEW() \
    {}
#define N1_FREE() \
    {}
#define N2_NEW() \
    {}
#define N2_FREE() \
    {}
#endif

void string_recycler_o::free_print() {
    for (auto p = p_free; p; p = p->p_next) {
        X_TRACE(("TRACE: %p : %p -- string on free list\n", this, p));
    }
}

void string_recycler_o::page_add() {
    auto p1 = (char*) ::pvalloc(n_size_page);
    auto p3 = p1 + n_size_page;
    for (auto p2 = p1; p2 < p3; p2 += n_size_small) {
        auto p = (recycled_buffer_p) p2;
        p->p_next = p_free;
        p_free = p;
    }
    X_TRACE(("TRACE: %p : %p -- page add\n", this, p1));
    free_print();
}

string_recycler_o::~string_recycler_o() {
    X_TRACE(("TRACE: %p : %p -- list clean up\n", this, p_free));
    recycled_buffer_p p_pages = 0;
    unsigned mask = (n_size_page - 1);
    auto p_next = p_free;
    p_free = 0;
    while (p_next) {
        auto p = p_next;
        p_next = p->p_next;
        if (0 == (mask & (uintptr_t) p)) {
            p->p_next = p_pages;
            p_pages = p;
        }
        X_TRACE(("TRACE: %p : %p -- string forget\n", this, p));
    }
    while (p_pages) {
        auto p = p_pages;
        p_pages = p->p_next;
        ::free(p);
        X_TRACE(("TRACE: %p : %p -- page free\n", this, p));
    }
}

char* string_recycler_o::buffer_new() {
    N1_NEW();
    if (!p_free) {
        page_add();
    }
    auto p = p_free;
    p_free = p->p_next;
    X_TRACE(("TRACE: %p : %p -- allocate small string from free list\n", this, p));
    return (char*) p;
}

char* string_recycler_o::buffer_new(unsigned n) {
    N2_NEW();
    auto p = new char[n + 1];
    X_TRACE(("TRACE: %p : %p -- allocate small string from memory\n", this, p));
    return p;
}

void string_recycler_o::buffer_free(char* s, unsigned n) {
    if ((n + 1) == n_size_small) {
        X_TRACE(("TRACE: %p : %p -- recycle small string\n", this, s));
        N1_FREE();
        auto p = (recycled_buffer_p) s;
        p->p_next = p_free;
        p_free = p;
    } else {
        X_TRACE(("TRACE: %p : %p -- free large string\n", this, s));
        N2_FREE();
        delete s;
    }
}

//
//  Define the string recycler to be thread-local.
//  This obviates need for locking/mutex.
//

static thread_local string_recycler_o recycler;

// For debugging.
void string_o::scorecard_get(scorecard_o& o) {
    o = recycler.scores;
}

string_o::string_o() {
    p_buffer = recycler.buffer_new();
    n_room = recycler.n_size_small - 1;
}

string_o::string_o(const char* s) {
    p_buffer = recycler.buffer_new();
    n_room = recycler.n_size_small - 1;
    strcpy(s);
}

string_o::string_o(const string_o& s) {
    p_buffer = recycler.buffer_new();
    n_room = recycler.n_size_small - 1;
    strcpy(s);
}

string_o::~string_o() {
    recycler.buffer_free(p_buffer, n_room);
    p_buffer = 0;
    n_room = 0;
}

inline char* string_o::ensure_room(unsigned n) {
    if (n <= n_room) {
        return p_buffer;
    }
    return expand_room(n);
}

char* base_strings::string_o::expand_room(unsigned n) {
    auto p1 = p_buffer;
    auto p2 = recycler.buffer_new(n);
    ::strcpy(p2, p1);
    recycler.buffer_free(p_buffer, n_room);
    p_buffer = p2;
    n_room = n;
    return p_buffer;
}

void string_o::strcpy(const char* s) {
    unsigned n = ::strlen(s);
    *p_buffer = 0;
    ::strcpy(ensure_room(n), s);
}

void string_o::strcpy(const char* s, unsigned n) {
    *p_buffer = 0;
    ::strncpy(ensure_room(n), s, n);
    p_buffer[n] = 0;
}

void string_o::strcat(const char* s) {
    unsigned n1 = ::strlen(p_buffer);
    unsigned n2 = ::strlen(s);
    ensure_room(n1 + n2);
    ::strcpy(p_buffer + n1, s);
}

void string_o::strcat(const char* s, unsigned n2) {
    unsigned n1 = ::strlen(p_buffer);
    ensure_room(n1 + n2);
    ::strncpy(p_buffer + n1, s, n2);
    p_buffer[n1 + n2] = 0;
}
