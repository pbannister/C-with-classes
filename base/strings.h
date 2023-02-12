#pragma once
#include <string.h>
#include <strings.h>

#define WANT_STRING_SCORECARD 0

namespace base_strings {

//
//  C-with-classes style lightweight and very-efficient string class.
//  Meant to be used with standard C string functions.
//  Not meant to offer every imaginable feature.
//  Uses a thread-safe free list for efficient allocation.
//

class string_o {
protected:
    char* p_buffer = 0;
    unsigned n_room = 0;

public:
    operator const char*() const {
        return p_buffer;
    }
    void operator=(const char* s) {
        strcpy(s);
    }
    void operator=(const string_o& s) {
        strcpy(s);
    }

public:
    char* buffer_get() {
        return p_buffer;
    }
    char* buffer_get(unsigned n) {
        return ensure_room(n);
    }

public:
    unsigned strlen() const {
        return ::strlen(p_buffer);
    }

public:
    void strcpy(const char*);
    void strcpy(const char*, unsigned);
    void strcat(const char*);
    void strcat(const char*, unsigned);

protected:
    char* ensure_room(unsigned);
    char* expand_room(unsigned);

public:
    // For debugging.
    struct scorecard_o {
        unsigned n1_new = 0;
        unsigned n1_free = 0;
        unsigned n2_new = 0;
        unsigned n2_free = 0;
    };
    static void scorecard_get(scorecard_o&);

public:
    string_o();
    string_o(const char*);
    string_o(const string_o&);
    ~string_o();
};

};  // namespace base_strings
