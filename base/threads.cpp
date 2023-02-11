#include "threads.h"

using namespace base_threads;

thread_o::thread_o() {
    error = ::pthread_attr_init(&thread_attributes);
}

thread_o::~thread_o() {
    error = ::pthread_attr_destroy(&thread_attributes);
}

bool thread_o::thread_create(void* (*pfn_start)(void*), void* pv_arg) {
    error = ::pthread_create(&thread_id, &thread_attributes, pfn_start, pv_arg);
    return (0 == error);
}

bool thread_o::thread_join(void** ppv) {
    error = ::pthread_join(thread_id, ppv);
    // Assume thread id not usable after this point.
    thread_id = 0;  // This is debatable.
    return (0 == error);
}
