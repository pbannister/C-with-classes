#include "strings.h"
#include <atomic>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

extern bool is_verbose(int n = 1);

using namespace base_strings;

//
//  Note the main thread is identified by identity, not by first use:
//  whichever thread happens to touch a string first is not necessarily
//  the main thread.
//
//  Static initialization (of id_thread_main, below) runs on the main
//  thread before main(), so the captured id is the main thread.
//

static const pthread_t id_thread_main = ::pthread_self();

static bool is_thread_main() {
    return (0 != ::pthread_equal(id_thread_main, ::pthread_self()));
}

unsigned thread_local_id_get() {
    // A monotonic process-local counter, so an id is never reused.
    // Neither pthread_self() nor gettid() is durable: both are recycled.
    static thread_local unsigned id_thread = 0;
    if (0 == id_thread) [[unlikely]] {
        static std::atomic<unsigned> id_next{1};
        id_thread = id_next.fetch_add(1, std::memory_order_relaxed);
    }
    return id_thread;
}

//
//  Strings of small/normal size are recycled for this string class.
//  Note this allocates small strings from pages, so less heap traffic.
//

struct recycler_string_local_o {
    unsigned id_thread = thread_local_id_get();
    const unsigned n_size_page = (1 << 12);
    const unsigned n_size_small = (1 << 7);

    struct recycled_buffer_o;
    typedef recycled_buffer_o* recycled_buffer_p;
    struct recycled_buffer_o {
        recycled_buffer_p p_next;
    };

    recycled_buffer_p p_free = 0;
    string_local_o::scorecard_o scores;  // for debugging

    void free_print();
    void page_add();

    char* buffer_new();
    char* buffer_new(unsigned n);
    void buffer_free(char* s, unsigned n);

    char* buffer_needed(unsigned n) {
        return ((n_size_small <= n) ? buffer_new(n) : buffer_new());
    }

    recycler_string_local_o() {
        page_add();
    }
    ~recycler_string_local_o();
};

void recycler_string_local_o::free_print() {
    for (auto p = p_free; p; p = p->p_next) {
        // ::printf("TRACE: %p : %p -- string on free list\n", this, p);
    }
}

void recycler_string_local_o::page_add() {
    auto p1 = (char*) ::pvalloc(n_size_page);
    auto p3 = p1 + n_size_page;
    for (auto p2 = p1; p2 < p3; p2 += n_size_small) {
        auto p = (recycled_buffer_p) p2;
        p->p_next = p_free;
        p_free = p;
    }
    free_print();
}

recycler_string_local_o::~recycler_string_local_o() {
    if (is_thread_main()) {
        // Do not free pages: objects with string members at namespace
        // scope are destroyed after the main thread's thread_local.
        return;
    }
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
    }
    while (p_pages) {
        auto p = p_pages;
        p_pages = p->p_next;
        ::free(p);
    }
    id_thread = 0;  // later string_local_o objects cannot use this.
}

char* recycler_string_local_o::buffer_new() {
    if (!p_free) [[unlikely]] {
        page_add();
    }
    auto p = p_free;
    p_free = p->p_next;
    auto s = (char*) p;
    *s = 0;
    return s;
}

char* recycler_string_local_o::buffer_new(unsigned n) {
    auto p = new char[n + 1];
    return p;
}

void recycler_string_local_o::buffer_free(char* s, unsigned n) {
    if ((n + 1) == n_size_small) [[likely]] {
        auto p = (recycled_buffer_p) s;
        p->p_next = p_free;
        p_free = p;
    } else [[unlikely]] {
        delete[] s;
    }
}

//
//  Define the string recycler to be thread-local.
//  This obviates need for locking/mutex.
//

static thread_local recycler_string_local_o recycler;

// For debugging.
void string_local_o::scorecard_get(scorecard_o& o) {
    o = recycler.scores;
}

string_local_o::string_local_o() {
    id_thread = recycler.id_thread;  // used only for identity.
    n_room = recycler.n_size_small - 1;
    p_buffer = recycler.buffer_new();
}

string_local_o::string_local_o(const char* s) {
    id_thread = recycler.id_thread;  // used only for identity.
    n_room = recycler.n_size_small - 1;
    p_buffer = recycler.buffer_new();
    strcpy(s);
}

string_local_o::string_local_o(const string_local_o& s) {
    id_thread = recycler.id_thread;  // used only for identity.
    n_room = recycler.n_size_small - 1;
    p_buffer = recycler.buffer_new();
    strcpy(s);
}

string_local_o::~string_local_o() {
    if (id_thread != recycler.id_thread) [[unlikely]] {
        // Better alternative?
        ::printf("string_local_o dispose by non-owner!\n");
    } else [[likely]] {
        recycler.buffer_free(p_buffer, n_room);
    }
    p_buffer = 0;
    n_room = 0;
}

inline char* string_local_o::ensure_room_have(unsigned n, const char* s1, unsigned n1) {
    if (n <= n_room) [[likely]] {
        return p_buffer;
    }
    return expand_room(n, s1, n1);
}

inline char* string_local_o::ensure_room_need(unsigned n, const char* s1, unsigned n1) {
    if (n <= n_room) [[likely]] {
        ::memcpy(p_buffer, s1, n1);
        p_buffer[n1] = 0;
        return p_buffer;
    }
    return expand_room(n, s1, n1);
}

char* string_local_o::expand_room(unsigned n, const char* s1, unsigned n1) {
    if (id_thread != recycler.id_thread) {
        throw exception_o("re-allocation of string by non-owner");
    }
    auto p2 = recycler.buffer_new(n);
    ::memcpy(p2, s1, n1);
    p2[n1] = 0;
    recycler.buffer_free(p_buffer, n_room);
    p_buffer = p2;
    n_room = n;
    return p_buffer;
}

void string_local_o::strcpy(const char* s1) {
    auto s2 = p_buffer;
    auto n1 = ::strlen(s1);
    auto n2 = (n_room - 1);
    auto p1 = (s1 + n1);
    auto p2 = (s2 + n2);
    if (p1 < s2) [[likely]] {
        // Source string is before destination string, so no overlap.
    } else if (p2 < s1) [[likely]] {
        // Source string is after destination string, so no overlap.
    } else [[unlikely]] {
        throw exception_o("source string overlaps destination string");
    }
    ensure_room_need(n1, s1, n1);
}

void string_local_o::strcpy(const char* s1, unsigned n1) {
    auto s2 = p_buffer;
    auto n2 = (n_room - 1);
    auto p1 = (s1 + n1);
    auto p2 = (s2 + n2);
    if (p1 < s2) [[likely]] {
        // Source string is before destination string, so no overlap.
    } else if (p2 < s1) [[likely]] {
        // Source string is after destination string, so no overlap.
    } else [[unlikely]] {
        throw exception_o("source string overlaps destination string");
    }
    ensure_room_need(n1, s1, n1);
}

void string_local_o::strcat(const char* s2) {
    auto s1 = p_buffer;
    auto n1 = ::strlen(s1);
    auto n2 = ::strlen(s2);
    auto p1 = (s1 + n1);
    auto p2 = (s2 + n2);
    if (p1 < s2) [[likely]] {
        // Destination string is before source string, so no overlap.
    } else if (p2 < s1) [[likely]] {
        // Source string is after destination string, so no overlap.
    } else [[unlikely]] {
        throw exception_o("source string overlaps destination string");
    }
    ensure_room_have(n1 + n2, s1, n1);
    ::memcpy(p_buffer + n1, s2, n2);
    p_buffer[n1 + n2] = 0;
}

void string_local_o::strcat(const char* s2, unsigned n2) {
    auto s1 = p_buffer;
    auto n1 = ::strlen(s1);
    auto p1 = (s1 + n1);
    auto p2 = (s2 + n2);
    if (p1 < s2) [[likely]] {
        // Destination string is before source string, so no overlap.
    } else if (p2 < s1) [[likely]] {
        // Source string is after destination string, so no overlap.
    } else [[unlikely]] {
        throw exception_o("source string overlaps destination string");
    }
    ensure_room_have(n1 + n2, s1, n1);
    ::memcpy(p_buffer + n1, s2, n2);
    p_buffer[n1 + n2] = 0;
}
