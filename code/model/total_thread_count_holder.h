#ifndef DIPLOM_TOTAL_THREAD_COUNT_HOLDER_H
#define DIPLOM_TOTAL_THREAD_COUNT_HOLDER_H

#include <cstdint>

/**
 * Holds total number of threads in the system. It is recommended to use this class
 * with global_storage<T> to be able to retrieve total number of threads in the system from
 * each of the threads.
 */
struct total_thread_count_holder
{
    /**
     * Total number of threads in the system.
     */
    const uint32_t total_thread_count;

    total_thread_count_holder(uint32_t count);
};

#endif //DIPLOM_TOTAL_THREAD_COUNT_HOLDER_H
