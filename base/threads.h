#pragma once
#include <pthread.h>

namespace base_threads {

//
//  C-with-classes style lightweight thread class.
//  Meant to be used with standard pthread functions.
//  Not meant to offer every imaginable feature.
//  Waits for child thread to exit in
//  Subclass to specialize.
//

class thread_owner_o {
protected:
    int error = 0;

protected:
    pthread_t thread_id = 0;
    pthread_attr_t thread_attributes;

public:
    thread_owner_o();
    ~thread_owner_o();

public:
    bool thread_create(void* (*) (void*), void*);
    bool thread_join(void**);
    bool thread_cancel();
};

}  // namespace base_threads
