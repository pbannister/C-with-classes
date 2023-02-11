#pragma once
#include <pthread.h>

namespace base_threads {

//
//
//

class thread_o {
protected:
    int error = 0;

protected:
    pthread_t thread_id = 0;
    pthread_attr_t thread_attributes;

public:
    thread_o();
    ~thread_o();

public:
    // Called by parent.
    bool thread_create(void* (*) (void*), void*);
    bool thread_join(void**);
};

}  // namespace base_threads
