#include "strings.h"
#include <stdio.h>

extern bool is_verbose(int n = 1);

// #define X_TRACE(L) 0
#define X_TRACE(L) { if (is_verbose()) { ::printf L ; } }

using namespace base_strings;

//
//  Strings of small/normal size are recycled for this string class.
//  TODO: Could extend this to allocate strings from pages.
//

struct string_recycler_o {
    struct recycled_buffer_o;
    typedef recycled_buffer_o* recycled_buffer_p;
    struct recycled_buffer_o {
        recycled_buffer_p p_next;
    };
    const unsigned n_size = (1 << 7);
    recycled_buffer_p p_free = 0;
    string_o::scorecard_o scores; // for debugging
    ~string_recycler_o() {
        X_TRACE(("TRACE: %p : %p -- list clean up\n", this, p_free));
        auto p_next = p_free;
        p_free = 0;
        while (p_next) {
            auto p = p_next;
            p_next = p->p_next;
            delete p;
            X_TRACE(("TRACE: %p : %p -- string clean up free\n", this, p));
        }
    }
    char* buffer_new() {
        scores.n1_new++;
        if (p_free) {
            auto p = p_free;
            p_free = p->p_next;
            X_TRACE(("TRACE: %p : %p -- allocate small string from free list\n", this, p));
            return (char*) p;
        } else {
            auto p = new char[n_size];
            X_TRACE(("TRACE: %p : %p -- allocate small string from memory\n", this, p));
            return p;
        }
    }
    char* buffer_new(unsigned n) {
        scores.n2_new++;
        auto p = new char[n + 1];
        X_TRACE(("TRACE: %p : %p -- allocate small string from memory\n", this, p));
        return p;
    }
    char* buffer_needed(unsigned n) {
        return ((n_size <= n) ? buffer_new(n) : buffer_new());
    }
    void buffer_free(char* s, unsigned n) {
        if ((n + 1) == n_size) {
            X_TRACE(("TRACE: %p : %p -- recycle small string\n", this, s));
            scores.n1_free++;
            auto p = (recycled_buffer_p) s;
            p->p_next = p_free;
            p_free = p;
        } else {
            X_TRACE(("TRACE: %p : %p -- free large string\n", this, s));
            scores.n2_free++;
            delete s;
        }
    }
};

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
    n_room = recycler.n_size - 1;
}

string_o::string_o(const char* s) {
    p_buffer = recycler.buffer_new();
    n_room = recycler.n_size - 1;
    strcpy(s);
}

string_o::string_o(const string_o& s) {
    p_buffer = recycler.buffer_new();
    n_room = recycler.n_size - 1;
    strcpy(s);
}

string_o::~string_o() {
    recycler.buffer_free(p_buffer, n_room);
    p_buffer = 0;
    n_room = 0;
}

char* string_o::ensure_room(unsigned n) {
    if (n <= n_room) {
        return p_buffer;
    }
    auto p1 = p_buffer;
    auto p2 = recycler.buffer_new(n);
    ::strcpy(p2, p1);
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
